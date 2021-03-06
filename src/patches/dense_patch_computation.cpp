#include <lsqecc/patches/dense_patch_computation.hpp>

namespace lsqecc
{


DenseSlice first_dense_slice_from_layout(const Layout& layout, const tsl::ordered_set<PatchId>& core_qubit_ids)
{
    DenseSlice slice(layout);

    if (layout.core_patches().size()<core_qubit_ids.size())
        throw std::runtime_error("Not enough Init patches for all ids");

    auto core_qubit_ids_itr = core_qubit_ids.begin();
    for (const SparsePatch& p : layout.core_patches())
    {
        Cell cell = slice.place_sparse_patch(p);
        slice.patch_at(cell)->id = *core_qubit_ids_itr++;
    }

    for(const MultipleCellsOccupiedByPatch& distillation_region: layout.distillation_regions())
    {
        for (const SingleCellOccupiedByPatch& cell: distillation_region.sub_cells)
        {
            slice.patch_at(cell.cell) = DensePatch{
                Patch{PatchType::Distillation,PatchActivity::Distillation,std::nullopt},
                static_cast<CellBoundaries>(cell)};
        }
    }

    size_t distillation_time_offset = 0;
    for(auto t : layout.distillation_times())
        slice.time_to_next_magic_state_by_distillation_region.push_back(t+distillation_time_offset++);

    return slice;
}

std::optional<Cell> find_place_for_magic_state(const DenseSlice& slice, const Layout& layout, size_t distillation_region_idx)
{
    for(const auto& cell: layout.distilled_state_locations(distillation_region_idx))
        if(slice.is_cell_free(cell))
            return cell;

    return std::nullopt;
}

std::optional<Cell> find_free_ancilla_location(const Layout& layout, const DenseSlice& slice)
{
    for(const Cell& possible_ancilla_location : layout.ancilla_location())
        if(slice.is_cell_free(possible_ancilla_location))
            return possible_ancilla_location;
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
        time_to_magic_state_here--;

        if(time_to_magic_state_here == 0){

            auto magic_state_cell = find_place_for_magic_state(slice, layout, distillation_region_index);
            if(magic_state_cell)
            {
                SparsePatch magic_state_patch = LayoutHelpers::basic_square_patch(*magic_state_cell);
                magic_state_patch.type = PatchType::PreparedState;
                slice.place_sparse_patch(magic_state_patch);
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
        slice.place_sparse_patch(SparsePatch{{PatchType::Routing,PatchActivity::None},occupied_cell});
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


// TODO replace with a variant
struct InstructionApplicationResult
{
    std::unique_ptr<std::exception> maybe_error;
    std::vector<LSInstruction> followup_instructions;
};

InstructionApplicationResult try_apply_instruction(
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
            return {nullptr, {}};
        }
    }
    else if (const auto* m = std::get_if<MultiPatchMeasurement>(&instruction.operation))
    {

        if (m->observable.size()!=2)
            throw std::logic_error("Multi patch measurement only supports 2 patches currently");
        auto pairs = m->observable.begin();
        const auto&[source_id, source_op] = *pairs++;
        const auto&[target_id, target_op] = *pairs;

        if (!slice.has_patch(source_id))
            return {std::make_unique<std::runtime_error>(lstk::cat("Patch ", source_id, " not on lattice")), {}};
        if (!slice.has_patch(target_id))
            return {std::make_unique<std::runtime_error>(lstk::cat("Patch ", target_id, " not on lattice")), {}};

        if (!merge_patches(slice, router, source_id, source_op, target_id, target_op))
            return {std::make_unique<std::runtime_error>(lstk::cat("Couldn't find room to route: ",
                    source_id, ":", PauliOperator_to_string(source_op), ",",
                    target_id, ":", PauliOperator_to_string(target_op))), {}};
        return {nullptr, {}};
    }
    else if (const auto* init = std::get_if<PatchInit>(&instruction.operation))
    {
        auto location = find_free_ancilla_location(layout, slice);
        if (!location) return {std::make_unique<std::runtime_error>("Could not allocate ancilla"), {}};

        slice.patch_at(*location);
        slice.place_sparse_patch(LayoutHelpers::basic_square_patch(*location));
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
        else if (mr->wait_at_most_for<=0)
            return {std::make_unique<std::runtime_error>(
                    std::string{"Could not get magic state after waiting"}), {}};

        else
            return {nullptr, {{MagicStateRequest{mr->target, mr->wait_at_most_for-1}}}};

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

            slice.place_sparse_patch(busy_region->state_after_clearing);
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

    return {std::make_unique<std::runtime_error>("Unhandled LS instruction in PatchComputation"),{}};
}



DensePatchComputationResult run_through_dense_slices(
        LSInstructionStream&& instruction_stream,
        const Layout& layout,
        Router& router,
        std::optional<std::chrono::seconds> timeout,
        const DenseSliceVisitor& slice_visitor,
        bool graceful)
{

    DensePatchComputationResult res;

    auto run = [&]()
    {
        DenseSlice slice = first_dense_slice_from_layout(layout, instruction_stream.core_qubits());

        auto start = std::chrono::steady_clock::now();

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

            auto application_result = try_apply_instruction(slice, instruction, layout, router);
            if (application_result.maybe_error)
            {
                slice_visitor(slice);
                advance_slice(slice, layout);
                res.slice_count_++;

                application_result = try_apply_instruction(slice, instruction, layout, router);
                if (application_result.maybe_error)
                {
                    slice_visitor(slice);
                    throw std::runtime_error{application_result.maybe_error->what()};
                }
            }

            for (auto&& i: application_result.followup_instructions)
                future_instructions.push(i);

            if (timeout && lstk::since(start)>*timeout)
            {
                auto timeout_str = std::string{"Out of time after "}+std::to_string(timeout->count())+std::string{"s. "}
                        +std::string{"Consumed "}+std::to_string(res.ls_instructions_count_)+std::string{" Instructions. "}
                        +std::string{"Generated "}+std::to_string(res.slice_count_)+std::string{"Slices."};

                throw std::runtime_error{timeout_str};
            }
        }
        slice_visitor(slice);
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