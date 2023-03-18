#include <lsqecc/patches/dense_patch_computation.hpp>
#include <lsqecc/dag/domain_dags.hpp>

namespace lsqecc
{


std::optional<Cell> find_place_for_magic_state(const DenseSlice& slice, const Layout& layout, size_t distillation_region_idx)
{
    for(const auto& cell: layout.distilled_state_locations(distillation_region_idx))
        if (layout.magic_states_reserved()) {
            if (slice.patch_at(cell)->type == PatchType::Distillation)
                return cell;
        }
        else {
            if(slice.is_cell_free(cell))
                return cell;
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

        if(p->type == PatchType::Routing || p->activity == PatchActivity::Measurement)
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
                SparsePatch magic_state_patch = LayoutHelpers::basic_square_patch(*magic_state_cell);
                magic_state_patch.type = PatchType::PreparedState;
                slice.place_sparse_patch(magic_state_patch, true);
                slice.magic_state_queue.push(*magic_state_cell);
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
    for(const auto& occupied_cell : routing_region.cells)
        slice.place_sparse_patch(SparsePatch{{PatchType::Routing,PatchActivity::None},occupied_cell}, false);
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


std::vector<LSInstruction> followup_next_attempt(const LSInstruction& instruction)
{
    LSInstruction followup{instruction};
    if(instruction.wait_at_most_for == 0) return {};
    followup.wait_at_most_for--;
    return {followup};
}


// TODO replace with a variant
struct InstructionApplicationResult
{
    std::unique_ptr<std::exception> maybe_error;
    std::vector<LSInstruction> followup_instructions;
};



InstructionApplicationResult try_apply_instruction_direct_followup(
        DenseSlice& slice,
        LSInstruction instruction,
        const Layout& layout,
        Router& router)
{
    if (const auto* s = std::get_if<SinglePatchMeasurement>(&instruction.operation))
    {

        auto maybe_target_patch = slice.get_patch_by_id(s->target);
        if(!maybe_target_patch)
            return {std::make_unique<std::runtime_error>(lstk::cat("Patch ", s->target, " not on lattice")), {}};
        auto& target_patch = maybe_target_patch->get();

        if (target_patch.is_active())
            return {std::make_unique<std::runtime_error>(lstk::cat("Patch ", s->target, " is active")), {}};
        target_patch.activity = PatchActivity::Measurement;
        return {nullptr, {}};
    }
    else if (const auto* p = std::get_if<SingleQubitOp>(&instruction.operation))
    {
        auto maybe_target_patch = slice.get_patch_by_id(p->target);
        if (!maybe_target_patch)
            return {std::make_unique<std::runtime_error>(lstk::cat("Patch ", p->target, " not on lattice")), {}};
        auto& target_patch = maybe_target_patch->get();

        if (p->op==SingleQubitOp::Operator::S)
        {
            if (!merge_patches(slice, router, p->target, PauliOperator::X, p->target, PauliOperator::Z))
                return {std::make_unique<std::runtime_error>(lstk::cat("Could not do S gate routing on ", p->target)),
                        {}};
            LSInstruction corrective_term{SingleQubitOp{p->target, SingleQubitOp::Operator::Z}};
            return {nullptr, {corrective_term}};
        }
        else
        {
            if (target_patch.is_active())
                return {std::make_unique<std::runtime_error>(lstk::cat("Patch ", p->target, " is active")), {}};

            target_patch.activity = PatchActivity::Unitary;
            if (p->op == SingleQubitOp::Operator::H)
                target_patch.boundaries.instant_rotate();
                
            return {nullptr, {}};
        }
    }
    else if (const auto* m = std::get_if<MultiPatchMeasurement>(&instruction.operation))
    {

        if (m->observable.size()!=2)
            throw std::logic_error(lstk::cat("Multi patch measurement only supports 2 patches currently. Got:\n", *m));
        auto pairs = m->observable.begin();
        const auto&[source_id, source_op] = *pairs++;
        const auto&[target_id, target_op] = *pairs;

        if (!slice.has_patch(source_id))
            return {std::make_unique<std::runtime_error>(lstk::cat("Patch ", source_id, " not on lattice")), {}};
        if (!slice.has_patch(target_id))
            return {std::make_unique<std::runtime_error>(lstk::cat("Patch ", target_id, " not on lattice")), {}};

        if (!merge_patches(slice, router, source_id, source_op, target_id, target_op))
            return {std::make_unique<std::runtime_error>(lstk::cat("Couldn't find room to route:\n", *m)), {}};
        return {nullptr, {}};
    }
    else if (const auto* init = std::get_if<PatchInit>(&instruction.operation))
    {
        auto location= init->place_next_to ?
                  place_ancilla_next_to(slice, init->place_next_to->first, init->place_next_to->second)
                : find_free_ancilla_location(layout, slice);
        if (!location) return {std::make_unique<std::runtime_error>("Could not allocate ancilla"), {}};

        slice.patch_at(*location);
        slice.place_sparse_patch(LayoutHelpers::basic_square_patch(*location), false);
        slice.patch_at(*location)->id = init->target;

        return {nullptr, {}};
    }
    else if (const auto* rotation = std::get_if<RotateSingleCellPatch>(&instruction.operation))
    {
        if (!slice.has_patch(rotation->target))
            return {std::make_unique<std::runtime_error>(lstk::cat("Patch ", rotation->target, " not on lattice")), {}};
        const Cell target_cell = *slice.get_cell_by_id(rotation->target);

        std::optional<Cell> free_neighbour;

        for (auto neighbour_cell: slice.get_neigbours_within_slice(target_cell))
            if (slice.is_cell_free(neighbour_cell))
                free_neighbour = neighbour_cell;

        if (!free_neighbour)
            return {std::make_unique<std::runtime_error>(lstk::cat(
                    "Cannot rotate patch ", rotation->target, ": has no free neighbour")), {}};

        auto stages{LayoutHelpers::single_patch_rotation_a_la_litinski(
                slice.patch_at(target_cell)->to_sparse_patch(target_cell), *free_neighbour)};

        slice.patch_at(target_cell) = std::nullopt;
        apply_routing_region(slice,stages.stage_1);

        return {nullptr, {{BusyRegion{std::move(stages.stage_2), 1, std::move(stages.final_state)}}}};
    }
    else if (auto* mr = std::get_if<MagicStateRequest>(&instruction.operation))
    {
        const auto& d_times = layout.distillation_times();
        if (!d_times.size()) throw std::logic_error("No distillation times");

        if (slice.magic_state_queue.size()>0)
        {
            const Cell newly_bound_magic_state_cell = lstk::queue_pop(slice.magic_state_queue);
            auto& newly_bound_magic_state = slice.patch_at(newly_bound_magic_state_cell).value();
            newly_bound_magic_state.id = mr->target;
            newly_bound_magic_state.type = PatchType::Qubit;
            newly_bound_magic_state.activity = PatchActivity::None;
            return {nullptr, {}};
        }
        else
            return {std::make_unique<std::runtime_error>(
                    std::string{"Could not get magic state"}), {}};

    }
    else if (auto* busy_region = std::get_if<BusyRegion>(&instruction.operation))
    {
        if(busy_region->steps_to_clear <= 0)
        {
            bool could_not_find_space_for_patch = false;
            busy_region->state_after_clearing.visit_individual_cells(
                    [&](const SingleCellOccupiedByPatch& occupied_cell){
                        if(!slice.is_cell_free(occupied_cell.cell))
                            could_not_find_space_for_patch = true;
                    });

            if(could_not_find_space_for_patch)
                return {std::make_unique<std::runtime_error>(
                        "Could not find space to place patch after rotation"),{std::move(instruction)}};
            slice.place_sparse_patch(busy_region->state_after_clearing, false);
            return {nullptr,{}};
        }
        else
        {
            for(const auto& occupied_cell: busy_region->region.cells)
                if(!slice.is_cell_free(occupied_cell.cell))
                    return {std::make_unique<std::runtime_error>("Could not find free cell for rotation"),
                            {std::move(instruction)}};

            apply_routing_region(slice, busy_region->region);
            return {nullptr,{{BusyRegion{
                    std::move(busy_region->region),
                    busy_region->steps_to_clear-1,
                    std::move(busy_region->state_after_clearing)}}}};
        }

    }

    std::stringstream s;
    s << "Unhandled LS instruction in PatchComputation: " << instruction;
    return {std::make_unique<std::runtime_error>(s.str()),{}};
}


InstructionApplicationResult try_apply_instruction_with_followup_attempts(
        DenseSlice& slice,
        const LSInstruction& instruction,
        const Layout& layout,
        Router& router)
{
    InstructionApplicationResult r = try_apply_instruction_direct_followup(slice, instruction, layout, router);
    if (r.maybe_error && r.followup_instructions.empty())
        return InstructionApplicationResult{nullptr, followup_next_attempt(instruction)};
    return r;
}



void run_through_dense_slices_streamed(
        LSInstructionStream&& instruction_stream,
        const Layout& layout,
        Router& router,
        std::optional<std::chrono::seconds> timeout,
        const DenseSliceVisitor& slice_visitor,
        bool graceful,
        DensePatchComputationResult& res)
{
    DenseSlice slice{layout, instruction_stream.core_qubits()};
    std::queue<LSInstruction> future_instructions;

    while (instruction_stream.has_next_instruction() || !future_instructions.empty())
    {
        LSInstruction instruction = [&]()
        {
            if (!future_instructions.empty())
                return lstk::queue_pop(future_instructions);
            res.ls_instructions_count_++;
            return instruction_stream.get_next_instruction();
        }();

        auto application_result = try_apply_instruction_with_followup_attempts(slice, instruction, layout, router);
        if (!application_result.followup_instructions.empty())
        {
            slice_visitor(slice);
            advance_slice(slice, layout);
            res.slice_count_++;

            for (auto&& i: application_result.followup_instructions)
                future_instructions.push(i);
        }
        else if (application_result.maybe_error)
        {
            slice_visitor(slice);
            throw std::runtime_error{application_result.maybe_error->what()};
        }
    }
    slice_visitor(slice);
}


void run_through_dense_slices_dag(
        dag::DependencyDag<LSInstruction>&& dag,
        const tsl::ordered_set<PatchId>& core_qubits,
        const Layout& layout,
        Router& router,
        std::optional<std::chrono::seconds> timeout,
        const DenseSliceVisitor& slice_visitor,
        bool graceful,
        DensePatchComputationResult& res)
{
    DenseSlice slice{layout, core_qubits};

    std::unordered_map<dag::label_t, size_t> attempts_per_instruction;
    auto increment_attepmts = [&attempts_per_instruction](dag::label_t label)
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
            const LSInstruction& instruction = dag.at(instruction_label);
            auto application_result = try_apply_instruction_direct_followup(slice, instruction, layout, router);
            if (application_result.maybe_error)
                throw std::runtime_error{lstk::cat(
                    "Could not apply proximate instruction:\n",
                    instruction,"\n",
                    "Caused by:\n",
                    application_result.maybe_error->what())};
            else
                res.ls_instructions_count_++;
            
            handle_followup_instructions(instruction_label, std::move(application_result.followup_instructions));
            
        }

        // Now apply all non-proximate instructions, where possible
        auto non_proximate_instructions = dag.applicable_instructions();
        for (dag::label_t instruction_label: non_proximate_instructions)
        {
            const LSInstruction& instruction = dag.at(instruction_label);
            auto application_result = try_apply_instruction_direct_followup(slice, instruction, layout, router);
            if (application_result.maybe_error)
            {
                increment_attepmts(instruction_label);
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
                res.ls_instructions_count_++;

            handle_followup_instructions(instruction_label, std::move(application_result.followup_instructions));

        }

        // Advance the slice
        slice_visitor(slice);
        advance_slice(slice, layout);
        res.slice_count_++;

    }
}


DensePatchComputationResult run_through_dense_slices(
        LSInstructionStream&& instruction_stream,
        bool dag_pipeline,
        const Layout& layout,
        Router& router,
        std::optional<std::chrono::seconds> timeout,
        DenseSliceVisitor slice_visitor,
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
        if (dag_pipeline)
        {
            auto dag = dag::full_dependency_dag_from_instruction_stream(instruction_stream);
            return run_through_dense_slices_dag(
                std::move(dag),
                instruction_stream.core_qubits(),
                layout,
                router,
                timeout,
                slice_visitor,
                graceful,
                res);
        }
        else
        {
            return run_through_dense_slices_streamed(
                std::move(instruction_stream),
                layout,
                router,
                timeout,
                slice_visitor,
                graceful,
                res);
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