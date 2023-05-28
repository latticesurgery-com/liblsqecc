#include <lsqecc/patches/dense_patch_computation.hpp>
#include <lsqecc/dag/domain_dags.hpp>
#include <lsqecc/scheduler/wave_scheduler.hpp>

#include <algorithm>

namespace lsqecc
{


std::optional<Cell> find_place_for_magic_state(const DenseSlice& slice, const Layout& layout, size_t distillation_region_idx)
{
    for(const auto& cell: layout.distilled_state_locations(distillation_region_idx))
    {
        if (layout.magic_states_reserved()) 
        {   // Case where magic states queues are reserved.
            // TODO how do we know slice.patch_at(cell) is not null.
            if (slice.patch_at(cell)->type == PatchType::Distillation)
                return cell;
        }
        else 
        {   // Regular case where magic states are queued on the boundary of the distillation region.
            if(slice.is_cell_free(cell))
                return cell;
        }
    }
    return std::nullopt;
}

std::optional<Cell> find_free_ancilla_location(const Layout& layout, const DenseSlice& slice)
{
    for(const Cell& possible_ancilla_location : layout.ancilla_location())
        if(slice.is_cell_free(possible_ancilla_location))
            return possible_ancilla_location;
    return std::nullopt;
}

std::optional<Cell> place_ancilla_next_to(const DenseSlice& slice, PatchId target, PauliOperator boundary_op)
{
    Cell target_cell = slice.get_cell_by_id(target).value();
    for(const Cell& possible_ancilla_location : slice.get_neigbours_within_slice(target_cell))
    {
        auto boundary = slice.get_boundary_between(target_cell, possible_ancilla_location);
        if(boundary && boundary->get().boundary_type == boundary_for_operator(boundary_op) && slice.is_cell_free(possible_ancilla_location))
            return possible_ancilla_location;
    }
    return std::nullopt;
}

void advance_slice(DenseSlice& slice, const Layout& layout)
{
    slice.traverse_cells_mut([&](const Cell& c, std::optional<DensePatch>& p) {
        if(!p) return;
        if(p->activity == PatchActivity::Unitary)
            p->activity = PatchActivity::None;

        p->boundaries.top.is_active = false;
        p->boundaries.bottom.is_active = false;
        p->boundaries.left.is_active = false;
        p->boundaries.right.is_active = false;

        if((p->type == PatchType::Routing && p->activity != PatchActivity::Busy) || p->activity == PatchActivity::Measurement)
        {
            p = std::nullopt;
            return;
        }
    });

    size_t distillation_region_index = 0;
    for (auto& time_to_magic_state_here: slice.time_to_next_magic_state_by_distillation_region)
    {
        if (layout.magic_states_reserved()) {
            for (const Cell& cell: layout.distilled_state_locations(distillation_region_index)) {
                if (slice.is_cell_free(cell)) {
                    slice.patch_at(cell) = DensePatch{
                        Patch{PatchType::Distillation,PatchActivity::Distillation,std::nullopt},
                        CellBoundaries{Boundary{BoundaryType::Connected, false},Boundary{BoundaryType::Connected, false},
                            Boundary{BoundaryType::Connected, false},Boundary{BoundaryType::Connected, false}}};
                }
            }            
        }
        
        time_to_magic_state_here--;

        if(time_to_magic_state_here == 0){

            auto magic_state_cell = find_place_for_magic_state(slice, layout, distillation_region_index);
            if(magic_state_cell)
            {
                SparsePatch magic_state_patch = LayoutHelpers::basic_square_patch(*magic_state_cell, std::nullopt, "Magic State");
                magic_state_patch.type = PatchType::PreparedState;
                slice.place_sparse_patch(magic_state_patch, true);
                slice.magic_states.insert(*magic_state_cell);
            }
            time_to_magic_state_here = layout.distillation_times()[distillation_region_index];
        }

        distillation_region_index++;
    }
}


void stitch_boundaries(
        DenseSlice& slice,
        Cell source_cell,
        Cell target_cell,
        RoutingRegion& routing_region)
{
    if (routing_region.cells.empty())
    {
        slice.get_boundary_between(source_cell,target_cell)->get().is_active=true;
        slice.get_boundary_between(target_cell,source_cell)->get().is_active=true;
    }
    else
    {
        slice.get_boundary_between(source_cell,routing_region.cells.back().cell)->get().is_active=true;
        slice.get_boundary_between(target_cell,routing_region.cells.front().cell)->get().is_active=true;
    }
}


void apply_routing_region(DenseSlice& slice, const RoutingRegion& routing_region)
{
    for (const auto& occupied_cell : routing_region.cells)
        slice.place_sparse_patch(SparsePatch{{PatchType::Routing, PatchActivity::None},occupied_cell}, false);
}

void mark_busy_region(DenseSlice& slice, const RoutingRegion& routing_region)
{
    for (const auto& occupied_cell : routing_region.cells)
    {
        auto& patch = slice.patch_at(occupied_cell.cell);
        if (patch)
        {
            assert(patch->activity == PatchActivity::Busy);
        }
        else
        {
            slice.place_sparse_patch(SparsePatch{{PatchType::Routing, PatchActivity::Busy},occupied_cell}, false);
        }
        
    }
}

void clear_busy_region(DenseSlice& slice, const RoutingRegion& routing_region)
{
    for (const auto& occupied_cell : routing_region.cells)
    {
        auto cell = occupied_cell.cell;
        auto& patch = slice.patch_at(cell);
        assert(patch && patch->activity == PatchActivity::Busy);
        patch = std::nullopt;
    }
}


/*
 * Returns true iff merge was successful
 */
bool merge_patches(
        DenseSlice& slice,
        Router& router,
        PatchId source,
        PauliOperator source_op,
        PatchId target,
        PauliOperator target_op)
{

    // TODO remove duplicate cell/patch search
    if(slice.get_patch_by_id(source)->get().is_active() || slice.get_patch_by_id(target)->get().is_active())
        return false;

    auto routing_region = router.find_routing_ancilla(slice, source, source_op, target, target_op);
    if(!routing_region)
        return false;

    // TODO check that the path is actually free when caching

    stitch_boundaries(slice, *slice.get_cell_by_id(source), *slice.get_cell_by_id(target), *routing_region);
    apply_routing_region(slice, *routing_region);

    return true;
}


InstructionApplicationResult try_apply_local_instruction(
        DenseSlice& slice,
        LocalInstruction::LocalLSInstruction instruction,
        const Layout& layout,
        Router& router)
{

    if (const auto* bellprep = std::get_if<LocalInstruction::BellPrepare>(&instruction.operation))
    {
        if (!slice.is_cell_free(bellprep->cell1))
        {
            throw std::runtime_error(lstk::cat(instruction, "; Cell ", bellprep->cell1, " is not free, cannot prepare state"));
        }
        else if (!slice.is_cell_free(bellprep->cell2))
        {
            throw std::runtime_error(lstk::cat(instruction, "; Cell ", bellprep->cell2, " is not free, cannot prepare state"));
        }

        slice.place_sparse_patch(LayoutHelpers::basic_square_patch(bellprep->cell1, bellprep->side1, "Bell 1"), false);
        slice.place_sparse_patch(LayoutHelpers::basic_square_patch(bellprep->cell2, bellprep->side2, "Bell 2"), false);
        slice.get_boundary_between_or_fail(bellprep->cell1, bellprep->cell2).get().is_active=true;
        slice.get_boundary_between_or_fail(bellprep->cell2, bellprep->cell1).get().is_active=true;

        return {nullptr, {}};
    }

    else if (const auto* bellmeas = std::get_if<LocalInstruction::BellMeasure>(&instruction.operation))
    {
        if (!slice.patch_at(bellmeas->cell1).has_value())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; No Patch at ", bellmeas->cell1, ", cannot measure")), {}};
        else if (!slice.patch_at(bellmeas->cell2).has_value())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; No Patch at ", bellmeas->cell2, ", cannot measure")), {}};
        else if (slice.patch_at(bellmeas->cell1)->is_active())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Patch at ", bellmeas->cell1, " is active, cannot measure")), {}};
        else if (slice.patch_at(bellmeas->cell2)->is_active())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Patch at ", bellmeas->cell2, " is active, cannot measure")), {}};

        slice.get_boundary_between_or_fail(bellmeas->cell1,bellmeas->cell2).get().is_active=true;
        slice.get_boundary_between_or_fail(bellmeas->cell2,bellmeas->cell1).get().is_active=true;
        slice.patch_at(bellmeas->cell1).value().activity = PatchActivity::Measurement;
        slice.patch_at(bellmeas->cell2).value().activity = PatchActivity::Measurement;

        return {nullptr, {}};
    }

    else if (const auto* move = std::get_if<LocalInstruction::Move>(&instruction.operation))
    {
        if (!slice.patch_at(move->source_cell).has_value())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; No Patch at ", move->source_cell, ", cannot move")), {}};
        else if (slice.patch_at(move->source_cell)->is_active())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Patch at ", move->source_cell, " is active, cannot move")), {}};
        else if (!slice.is_cell_free(move->target_cell))
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Cell ", move->target_cell, " is not free, cannot move")), {}};
        
        SparsePatch new_patch = LayoutHelpers::basic_square_patch(move->target_cell, std::nullopt, "Move");
        new_patch.id = move->new_id_for_target ? move->new_id_for_target : slice.patch_at(move->source_cell)->id;
        slice.place_sparse_patch(new_patch, false);
        slice.get_boundary_between_or_fail(move->source_cell, move->target_cell).get().is_active=true;
        slice.get_boundary_between_or_fail(move->target_cell, move->source_cell).get().is_active=true;
        slice.patch_at(move->source_cell)->activity = PatchActivity::Measurement;
        slice.patch_at(move->source_cell)->id = std::nullopt;

        return {nullptr, {}};
    }
    else if (const auto* localmeas = std::get_if<LocalInstruction::TwoPatchMeasure>(&instruction.operation))
    {
        if (!slice.patch_at(localmeas->cell1).has_value())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; No Patch at ", localmeas->cell1, ", cannot measure")), {}};
        else if (!slice.patch_at(localmeas->cell2).has_value())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; No Patch at ", localmeas->cell2, ", cannot measure")), {}};
        else if (slice.patch_at(localmeas->cell1)->is_active())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Patch at ", localmeas->cell1, " is active, cannot measure")), {}};
        else if (slice.patch_at(localmeas->cell2)->is_active())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Patch at ", localmeas->cell2, " is active, cannot measure")), {}};

        slice.get_boundary_between_or_fail(localmeas->cell1,localmeas->cell2).get().is_active=true;
        slice.get_boundary_between_or_fail(localmeas->cell2,localmeas->cell1).get().is_active=true;

        return {nullptr, {}};
}
    else if (const auto* extendsplit = std::get_if<LocalInstruction::ExtendSplit>(&instruction.operation))
    {
        
        if (!slice.patch_at(extendsplit->target_cell).has_value())
            throw std::runtime_error(lstk::cat(instruction, "; No Patch at ", extendsplit->target_cell, ", cannot extend"));
        else if (slice.patch_at(extendsplit->target_cell)->is_active())
            throw std::runtime_error(lstk::cat(instruction, "; Patch at ", extendsplit->target_cell, " is active, cannot extend"));
        else if (!slice.is_cell_free(extendsplit->extension_cell))
            throw std::runtime_error(lstk::cat(instruction, "; Cell ", extendsplit->extension_cell, " is not free, cannot extend"));

        slice.place_sparse_patch(LayoutHelpers::basic_square_patch(extendsplit->extension_cell, extendsplit->extension_id, "Extended"), false);
        slice.get_boundary_between(extendsplit->extension_cell, extendsplit->target_cell)->get().is_active=true;
        slice.get_boundary_between(extendsplit->target_cell, extendsplit->extension_cell)->get().is_active=true;

        return {nullptr, {}};
    }
    else if (const auto* mergecont = std::get_if<LocalInstruction::MergeContract>(&instruction.operation))
    {
        if (!slice.patch_at(mergecont->preserved_cell).has_value())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; No Patch at ", mergecont->preserved_cell, ", cannot merge")), {}};
        else if (!slice.patch_at(mergecont->measured_cell).has_value())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; No Patch at ", mergecont->measured_cell, ", cannot merge")), {}};
        else if (slice.patch_at(mergecont->preserved_cell)->is_active())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Patch at ", mergecont->preserved_cell, " is active, cannot merge")), {}};
        else if (slice.patch_at(mergecont->measured_cell)->is_active())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Patch at ", mergecont->measured_cell, " is active, cannot merge")), {}};

        slice.get_boundary_between_or_fail(mergecont->preserved_cell,mergecont->measured_cell).get().is_active=true;
        slice.get_boundary_between_or_fail(mergecont->measured_cell,mergecont->preserved_cell).get().is_active=true;
        slice.patch_at(mergecont->measured_cell).value().activity = PatchActivity::Measurement;

        return {nullptr, {}};
    }
    LSTK_UNREACHABLE;
}

InstructionApplicationResult try_apply_instruction_direct_followup(
        DenseSlice& slice,
        LSInstruction& instruction,
        bool local_instructions,
        const Layout& layout,
        Router& router)
{
    if (const auto* s = std::get_if<SinglePatchMeasurement>(&instruction.operation))
    {
        auto maybe_target_patch = slice.get_patch_by_id(s->target);
        if(!maybe_target_patch)
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", s->target, " not on lattice")), {}};

        auto& target_patch = maybe_target_patch->get();
        target_patch.activity = PatchActivity::Measurement;
        
        return {nullptr, {}};
    }
    else if (const auto* p = std::get_if<SingleQubitOp>(&instruction.operation))
    {
        auto maybe_target_patch = slice.get_patch_by_id(p->target);
        if (!maybe_target_patch)
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", p->target, " not on lattice")), {}};
        auto& target_patch = maybe_target_patch->get();

        if (p->op==SingleQubitOp::Operator::S)
        {
            if (!merge_patches(slice, router, p->target, PauliOperator::X, p->target, PauliOperator::Z))
                return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Could not do S gate routing on ", p->target)),
                        {}};
            LSInstruction corrective_term{SingleQubitOp{p->target, SingleQubitOp::Operator::Z}};
            return {nullptr, {corrective_term}};
        }
        else
        {
            if (target_patch.is_active())
                return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", p->target, " is active")), {}};
            
            if (p->op == SingleQubitOp::Operator::H)
            {
                target_patch.activity = PatchActivity::Unitary;
                target_patch.boundaries.instant_rotate();
            }
                
            return {nullptr, {}};
        }
    }
    else if (const auto* m = std::get_if<MultiPatchMeasurement>(&instruction.operation))
    {

        if (m->observable.size()!=2)
            throw std::logic_error(lstk::cat(instruction,"; Multi patch measurement only supports 2 patches currently. Got:\n", *m));
        auto pairs = m->observable.begin();
        const auto&[source_id, source_op] = *pairs++;
        const auto&[target_id, target_op] = *pairs;

        if (!slice.has_patch(source_id))
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", source_id, " not on lattice")), {}};
        if (!slice.has_patch(target_id))
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", target_id, " not on lattice")), {}};

        if (!local_instructions) 
        {
            if (!merge_patches(slice, router, source_id, source_op, target_id, target_op))
                return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Couldn't find room to route")), {}};
            return {nullptr, {}};
        }
        else
        {
            // TODO: break up long-range ops into local ops
            std::vector<LocalInstruction::LocalLSInstruction> local_instructions;
            local_instructions.push_back({LocalInstruction::TwoPatchMeasure{slice.get_cell_by_id(source_id).value(), slice.get_cell_by_id(target_id).value()}});
            InstructionApplicationResult r = try_apply_local_instruction(slice, local_instructions[0], layout, router);
            if (r.maybe_error && r.followup_instructions.empty())
                return InstructionApplicationResult{std::move(r.maybe_error), {}};
            if (!r.followup_instructions.empty())
                return InstructionApplicationResult{std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Followup local instructions not implemented")), {}};
            return {nullptr, {}};
        }


    }
    else if (const auto* init = std::get_if<PatchInit>(&instruction.operation))
    {
        auto location= init->place_next_to ?
                  place_ancilla_next_to(slice, init->place_next_to->target, init->place_next_to->op)
                : find_free_ancilla_location(layout, slice);
        if (!location) return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Could not allocate ancilla")), {}};

        slice.patch_at(*location);
        slice.place_sparse_patch(LayoutHelpers::basic_square_patch(*location, std::nullopt, "Init"), false);
        slice.patch_at(*location)->id = init->target;

        return {nullptr, {}};
    }
    else if (auto* bell_init = std::get_if<BellPairInit>(&instruction.operation))
    {
        if (!slice.has_patch(bell_init->loc1.target))
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", bell_init->loc1.target, " not on lattice")), {}};
        if (!slice.has_patch(bell_init->loc2.target)) 
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", bell_init->loc2.target, " not on lattice")), {}};

        if (!local_instructions)
        {
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; not implemented for local instruction compilation")), {}};
        }
        else 
        {
            if (!bell_init->counter.has_value())
            {
                auto routing_region = router.find_routing_ancilla(slice, bell_init->loc1.target, bell_init->loc1.op, bell_init->loc2.target, bell_init->loc2.op);
                if(!routing_region)
                {
                    return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; No valid route found for Bell pair creation")), {}};
                }
                else if (routing_region->cells.size() < 2) 
                {
                    return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Shortest route cannot be used for Bell pair creation")), {}};
                }

                std::vector<LocalInstruction::LocalLSInstruction> local_instructions;
                local_instructions.reserve(routing_region->cells.size());
                std::optional<PatchId> id1; std::optional<PatchId> id2;
                for (size_t i=0; i<routing_region->cells.size()-1; i=i+2)
                {
                    // Push a BellPrepare instruction with PatchID's depending on the case
                    id1 = std::nullopt; id2 = std::nullopt;
                    if (i==0) 
                        id1 = bell_init->side2;
                    if (i==routing_region->cells.size()-2)
                        id2 = bell_init->side1;

                    local_instructions.push_back({LocalInstruction::BellPrepare{id1, id2, routing_region->cells[i].cell, routing_region->cells[i+1].cell}});
                }
                for (size_t i=2; i<routing_region->cells.size()-1; i=i+2)
                {
                    // Push a complementary layer of BellMeasure instructions
                    local_instructions.push_back({LocalInstruction::BellMeasure{routing_region->cells[i-1].cell, routing_region->cells[i].cell}});
                }
                // Take care of the case of an odd route
                if ((routing_region->cells.size()%2 == 1))
                {
                    local_instructions.push_back({LocalInstruction::Move{
                        routing_region->cells[routing_region->cells.size()-2].cell, 
                        routing_region->cells[routing_region->cells.size()-1].cell,
                        bell_init->side1
                    }});
                }

                bell_init->local_instructions = std::move(local_instructions);
                bell_init->counter = std::pair<unsigned int, unsigned int>(0, 0);
            }

            bell_init->counter->first = bell_init->counter->second;
            for (unsigned int i = bell_init->counter->first; i < bell_init->local_instructions.value().size(); i++)
            {
                InstructionApplicationResult r = try_apply_local_instruction(slice, bell_init->local_instructions.value()[i], layout, router);
                if (r.maybe_error && r.followup_instructions.empty())
                    return InstructionApplicationResult{nullptr, {instruction}};
                if (!r.followup_instructions.empty())
                    return InstructionApplicationResult{std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Followup local instructions not implemented")), {}};

                bell_init->counter->second++;
            }
            return {nullptr, {}};
        }
    }
    else if (auto* bell_cnot = std::get_if<BellBasedCNOT>(&instruction.operation))
    {
        if (!slice.has_patch(bell_cnot->control))
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", bell_cnot->control, " not on lattice")), {}};
        if (!slice.has_patch(bell_cnot->target)) 
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", bell_cnot->target, " not on lattice")), {}};
        
        auto control_patch = slice.get_patch_by_id(bell_cnot->control);
        auto target_patch = slice.get_patch_by_id(bell_cnot->target);
        
        if (control_patch.value().get().is_active())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", bell_cnot->control, " is active")), {}};
        if (target_patch.value().get().is_active())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", bell_cnot->target, " is active")), {}};
        
        if (!local_instructions)
        {
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; not implemented unless local instruction flag is given")), {}};
        }
        else 
        {
            if (!bell_cnot->counter.has_value())
            {
                auto routing_region = router.find_routing_ancilla(slice, bell_cnot->control, PauliOperator::Z, bell_cnot->target, PauliOperator::X);
                if(!routing_region) 
                {
                    return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; No valid route found for Bell pair creation")), {}};
                }
                else if (routing_region->cells.size() < 2) 
                {
                    return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Shortest route cannot be used for Bell pair creation")), {}};
                }

                std::vector<LocalInstruction::LocalLSInstruction> local_instructions;
                local_instructions.reserve(2*routing_region->cells.size());

                // Offset boolean for even vs. odd routes
                bool even_route = (routing_region->cells.size()%2 == 0);

                // See PRX Quantum 3, 020342 (2022) Fig. 19a and c for even vs. odd route compilation
                if (!even_route)
                {
                    local_instructions.push_back({LocalInstruction::ExtendSplit{
                        std::nullopt,
                        slice.get_cell_by_id(bell_cnot->control).value(), 
                        routing_region->cells[routing_region->cells.size()-1].cell
                    }});
                }
                std::optional<PatchId> id1; std::optional<PatchId> id2;
                for (size_t i=0; i<routing_region->cells.size()-1; i=i+2)
                {
                    // Push a BellPrepare instruction with PatchID's depending on the case
                    id1 = std::nullopt; id2 = std::nullopt;
                    if (i==0)
                        id1 = bell_cnot->side2;
                    if (i==routing_region->cells.size()-2-!even_route)
                        id2 = bell_cnot->side1;

                    local_instructions.push_back({LocalInstruction::BellPrepare{id1, id2, routing_region->cells[i].cell, routing_region->cells[i+1].cell}});
                }
                for (size_t i=2; i<routing_region->cells.size()-even_route; i=i+2)
                {
                    // Push a complementary layer of BellMeasure instructions
                    local_instructions.push_back({LocalInstruction::BellMeasure{routing_region->cells[i-1].cell, routing_region->cells[i].cell}});
                }
                // Add final measurements
                local_instructions.push_back({LocalInstruction::MergeContract{slice.get_cell_by_id(bell_cnot->target).value(), routing_region->cells[0].cell}});
                if (even_route)
                {
                    local_instructions.push_back({LocalInstruction::MergeContract{slice.get_cell_by_id(bell_cnot->control).value(), routing_region->cells[routing_region->cells.size()-1].cell}});                    
                }
                bell_cnot->local_instructions = std::move(local_instructions);
                bell_cnot->counter = std::pair<unsigned int, unsigned int>(0, 0);
            }
            bell_cnot->counter->first = bell_cnot->counter->second;
            
            for (unsigned int i = bell_cnot->counter->first; i < bell_cnot->local_instructions.value().size(); i++)
            {
                InstructionApplicationResult r = try_apply_local_instruction(slice, bell_cnot->local_instructions.value()[i], layout, router);
                if (r.maybe_error && r.followup_instructions.empty())
                    return InstructionApplicationResult{nullptr, {instruction}};
                if (!r.followup_instructions.empty())
                    return InstructionApplicationResult{std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Followup local instructions not implemented")), {}};

                bell_cnot->counter->second++;
            }
            return {nullptr, {}};
        }
    }
    else if (const auto* rotation = std::get_if<RotateSingleCellPatch>(&instruction.operation))
    {
        if (!slice.has_patch(rotation->target))
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", rotation->target, " not on lattice")), {}};
        const Cell target_cell = *slice.get_cell_by_id(rotation->target);

        std::optional<Cell> free_neighbour;

        for (auto neighbour_cell: slice.get_neigbours_within_slice(target_cell))
            if (slice.is_cell_free(neighbour_cell))
                free_neighbour = neighbour_cell;

        if (!free_neighbour)
            return {std::make_unique<std::runtime_error>(lstk::cat(
                    instruction,"; Cannot rotate patch ", rotation->target, ": has no free neighbour")), {}};

        if (slice.patch_at(target_cell)->activity == PatchActivity::Unitary)
        {
            slice.patch_at(target_cell)->activity = PatchActivity::None;
        }

        auto stages{LayoutHelpers::single_patch_rotation_a_la_litinski(
                slice.patch_at(target_cell)->to_sparse_patch(target_cell), *free_neighbour)};

        slice.patch_at(target_cell) = std::nullopt;
        mark_busy_region(slice,stages.stage_1);

        std::vector<SparsePatch> final_state{stages.final_state};

        return {nullptr, {{BusyRegion{std::move(stages.stage_2), 2, final_state}}}};
    }
    else if (auto* mr = std::get_if<MagicStateRequest>(&instruction.operation))
    {
        const auto& d_times = layout.distillation_times();
        if (!d_times.size()) throw std::logic_error("No distillation times");

        if (slice.magic_states.size()>0)
        {
            std::optional<Cell> min_cell;
            double min_dist = std::numeric_limits<double>::max();
            double dist;
            // 
            for (const Cell& cell : slice.magic_states)
            {
                dist = abs(cell.col - slice.get_cell_by_id(mr->near_patch).value().col) + abs(cell.row - slice.get_cell_by_id(mr->near_patch).value().row);
                if (dist < min_dist)
                {
                    min_dist = dist;
                    min_cell = cell;
                }
            }

            if (min_cell)
            {
                auto& newly_bound_magic_state = slice.patch_at(min_cell.value()).value();
                newly_bound_magic_state.id = mr->target;
                newly_bound_magic_state.type = PatchType::Qubit;
                newly_bound_magic_state.activity = PatchActivity::None;
                slice.magic_states.erase(min_cell.value());
                return {nullptr, {}};
            }
        }
        else 
        {
            return {std::make_unique<std::runtime_error>(
                    lstk::cat(instruction,";Could not get magic state")), {}};           
        }
    }
    else if (auto* yr = std::get_if<YStateRequest>(&instruction.operation))
    {
        // Unbind Y state patch if already bound
        if (slice.get_patch_by_id(yr->target))
        {
            slice.get_patch_by_id(yr->target)->get().id = std::nullopt;
            return{nullptr, {}};
        }
        // Otherwise, get minimum (L1) distance unbound Y state patch
        else 
        {
            std::optional<Cell> min_cell;
            double min_dist = std::numeric_limits<double>::max();
            double dist;
            // 
            for (const Cell& cell : layout.predistilled_y_states())
            {
                if (!slice.patch_at(cell)->id.has_value() && !slice.patch_at(cell)->is_active())
                {
                    dist = abs(cell.col - slice.get_cell_by_id(yr->near_patch).value().col) + abs(cell.row - slice.get_cell_by_id(yr->near_patch).value().row);
                    if (dist < min_dist)
                    {
                        min_dist = dist;
                        min_cell = cell;
                    }
                }
            }

            if (min_cell)
            {
                auto& patch = slice.patch_at(min_cell.value());
                if (patch)
                {
                    slice.patch_at(min_cell.value()).value().id = yr->target;
                    return {nullptr, {}};
                }
            }
            return {std::make_unique<std::runtime_error>(
                    std::string{"Could not get Y state"}), {}};           
        }

    }
    else if (auto* busy_region = std::get_if<BusyRegion>(&instruction.operation))
    {
        if(busy_region->steps_to_clear <= 0)
        {
            clear_busy_region(slice, busy_region->region);
            
            for (const SparsePatch& patch : busy_region->state_after_clearing) {
                bool could_not_find_space_for_patch = false;
                patch.visit_individual_cells(
                        [&](const SingleCellOccupiedByPatch& occupied_cell){
                            if(!slice.is_cell_free(occupied_cell.cell))
                                could_not_find_space_for_patch = true;
                        });

                if(could_not_find_space_for_patch)
                    return {std::make_unique<std::runtime_error>(lstk::cat(
                            instruction,"; Could not find space to place patch after BusyRegion clears")),{}};
                auto* single_cell = std::get_if<SingleCellOccupiedByPatch>(&patch.cells);
                if (single_cell) {
                    slice.place_sparse_patch(patch,false);
                }
                else {
                    slice.place_sparse_patch_multiple_cells(patch);
                }
            }

            return {nullptr,{}};
        }
        else
        {
            for(const auto& occupied_cell: busy_region->region.cells)
            {
                auto& patch = slice.patch_at(occupied_cell.cell);
                if(!patch && patch->activity != PatchActivity::Busy)
                {                    
                    return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Could not find free cell in BusyRegion")),
                            {instruction}};
                }
            }
            mark_busy_region(slice, busy_region->region);
            
            return {nullptr,{{BusyRegion{
                    busy_region->region,
                    busy_region->steps_to_clear-1,
                    busy_region->state_after_clearing}}}};
        }

    }
    else if (std::get_if<PatchReset>(&instruction.operation))
        return {nullptr, {}};
    
    LSTK_UNREACHABLE;
}

void run_through_dense_slices_streamed(
        LSInstructionStream&& instruction_stream,
        bool local_instructions,
        const Layout& layout,
        Router& router,
        std::optional<std::chrono::seconds> timeout,
        DenseSliceVisitor slice_visitor,
        LSInstructionVisitor instruction_visitor,
        bool graceful,
        DensePatchComputationResult& res)
{
    DenseSlice slice{layout, instruction_stream.core_qubits()};
    std::deque<LSInstruction> future_instructions;

    while (instruction_stream.has_next_instruction() || !future_instructions.empty())
    {
        LSInstruction instruction = [&]()
        {
            if (!future_instructions.empty())
                return lstk::deque_pop(future_instructions);
            res.ls_instructions_count_++;
            return instruction_stream.get_next_instruction();
        }();

        auto application_result = try_apply_instruction_direct_followup(slice, instruction, local_instructions, layout, router);
        if (!application_result.maybe_error)
            instruction_visitor(instruction);

        if (!application_result.followup_instructions.empty())
        {
            slice_visitor(slice);
            advance_slice(slice, layout);
            res.slice_count_++;

            for (auto&& i: application_result.followup_instructions)
                future_instructions.push_back(i);
        }
        else if (application_result.maybe_error)
        {
            slice_visitor(slice);
            if (instruction.wait_at_most_for == 0)
                throw std::runtime_error{application_result.maybe_error->what()};
            instruction.wait_at_most_for--;
            future_instructions.push_front(instruction);
            advance_slice(slice, layout);
        }
    }
    slice_visitor(slice);
}


void run_through_dense_slices_dag(
        dag::DependencyDag<LSInstruction>&& dag,
        const tsl::ordered_set<PatchId>& core_qubits,
        bool local_instructions,
        const Layout& layout,
        Router& router,
        std::optional<std::chrono::seconds> timeout,
        DenseSliceVisitor slice_visitor,
        LSInstructionVisitor instruction_visitor,
        bool graceful,
        DensePatchComputationResult& res)
{
    DenseSlice slice{layout, core_qubits};

    std::unordered_map<dag::label_t, size_t> attempts_per_instruction;
    auto increment_attempts = [&attempts_per_instruction](dag::label_t label)
    {
        if (!attempts_per_instruction.contains(label))
            attempts_per_instruction[label] = 0;
        attempts_per_instruction[label]++;
    };

    auto handle_followup_instructions = [&dag](dag::label_t instruction_label, std::vector<LSInstruction>&& followup_instructions)
    {
        if (!followup_instructions.empty())
            {
                dag::label_t new_head = dag.expand(instruction_label, std::move(followup_instructions), true);
                dag.make_proximate(new_head);
            }
            else
                dag.pop_head(instruction_label);
    };


    while (!dag.empty())
    {
        // Apply all proximate instructions
        auto proximate_instructions = dag.proximate_instructions();
        for (dag::label_t instruction_label: proximate_instructions)
        {
            LSInstruction& instruction = dag.at(instruction_label);
            auto application_result = try_apply_instruction_direct_followup(slice, instruction, local_instructions, layout, router);
            if (application_result.maybe_error)
                throw std::runtime_error{lstk::cat(
                    "Could not apply proximate instruction:\n",
                    instruction,"\n",
                    "Caused by:\n",
                    application_result.maybe_error->what())};
            else
            {
                res.ls_instructions_count_++;
                instruction_visitor(instruction);
            }
            handle_followup_instructions(instruction_label, std::move(application_result.followup_instructions));
            
        }

        // Now apply all non-proximate instructions, where possible
        auto non_proximate_instructions = dag.applicable_instructions();
        for (dag::label_t instruction_label: non_proximate_instructions)
        {
            LSInstruction& instruction = dag.at(instruction_label);
            auto application_result = try_apply_instruction_direct_followup(slice, instruction, local_instructions, layout, router);
            if (application_result.maybe_error)
            {
                increment_attempts(instruction_label);
                if (attempts_per_instruction[instruction_label] > MAX_INSTRUCTION_APPLICATION_RETRIES_DAG_PIPELINE)
                {
                    throw std::runtime_error{lstk::cat(
                        "Could not apply non-proximate instruction after ",
                        MAX_INSTRUCTION_APPLICATION_RETRIES_DAG_PIPELINE," retries:\n",
                        instruction,"\n",
                        "Caused by:\n",
                        application_result.maybe_error->what())};
                }
            }
            else
            {
                res.ls_instructions_count_++;
                instruction_visitor(instruction);
                handle_followup_instructions(instruction_label, std::move(application_result.followup_instructions));
            }
        }

        // Advance the slice
        slice_visitor(slice);
        advance_slice(slice, layout);
        res.slice_count_++;

    }
}

void run_through_dense_slices_wave(
        LSInstructionStream&& instruction_stream,
        bool local_instructions,
        const Layout& layout,
        Router& router,
        std::optional<std::chrono::seconds> timeout,
        DenseSliceVisitor slice_visitor,
        LSInstructionVisitor instruction_visitor,
        bool graceful,
        DensePatchComputationResult& res)
{
    DenseSlice slice{layout, instruction_stream.core_qubits()};
    WaveScheduler scheduler(std::move(instruction_stream), local_instructions, layout);
    
    while (!scheduler.done())
    {
        scheduler.schedule_wave(slice, instruction_visitor, res);
        
        slice_visitor(slice);
        advance_slice(slice, layout);
        
        res.slice_count_++;
    }
}


DensePatchComputationResult run_through_dense_slices(
        LSInstructionStream&& instruction_stream,
        PipelineMode pipeline_mode,
        bool local_instructions,
        const Layout& layout,
        Router& router,
        std::optional<std::chrono::seconds> timeout,
        DenseSliceVisitor slice_visitor,
        LSInstructionVisitor instruction_visitor,
        bool graceful)
{

    DensePatchComputationResult res;

    // TODO move this out of here and into the pipeline, if possible. (might have to also take res out)
    auto start = lstk::now();
    if(timeout.has_value())
    {

        slice_visitor = [&, slice_visitor](const DenseSlice& slice)
        {
            if (timeout && lstk::since(start)>*timeout)
            {
                auto timeout_str = std::string{"Out of time after "}+std::to_string(timeout->count())+std::string{"s. "}
                        +std::string{"Consumed "}+std::to_string(res.ls_instructions_count_)+std::string{" Instructions. "}
                        +std::string{"Generated "}+std::to_string(res.slice_count_)+std::string{"Slices."};

                throw std::runtime_error{timeout_str};
            }
            slice_visitor(slice);
        };
    }

    auto run = [&]()
    {
        switch (pipeline_mode)
        {
        case PipelineMode::Stream:
            return run_through_dense_slices_streamed(
                std::move(instruction_stream),
                local_instructions,
                layout,
                router,
                timeout,
                slice_visitor,
                instruction_visitor,
                graceful,
                res);
        
        case PipelineMode::Dag:
        {
            auto dag = dag::full_dependency_dag_from_instruction_stream(instruction_stream);
            return run_through_dense_slices_dag(
                std::move(dag),
                instruction_stream.core_qubits(),
                local_instructions,
                layout,
                router,
                timeout,
                slice_visitor,
                instruction_visitor,
                graceful,
                res);
        }
        
        case PipelineMode::Wave:
            return run_through_dense_slices_wave(
                std::move(instruction_stream),
                local_instructions,
                layout,
                router,
                timeout,
                slice_visitor,
                instruction_visitor,
                graceful,
                res);
        
        default: LSTK_UNREACHABLE;
        }
    };

    if(graceful)
    {
        try{
            run();
        }
        catch (const std::exception& e)
        {

            std::cout << "Encountered exception: " << e.what() << std::endl;
            std::cout << "Halting slicing" << std::endl;
        }
    }
    else
        run();

    return res;
}


DensePatchComputationResult::DensePatchComputationResult(const DensePatchComputationResult& other)
 : ls_instructions_count_(other.ls_instructions_count_), slice_count_(other.slice_count_)
{}

}
