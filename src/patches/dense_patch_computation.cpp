#include <lsqecc/patches/dense_patch_computation.hpp>

namespace lsqecc
{


DenseSlice first_slice_from_layout(const Layout& layout, const tsl::ordered_set<PatchId>& core_qubit_ids)
{
    DenseSlice slice = DenseSlice::make_blank_slice(layout);

    if (layout.core_patches().size()<core_qubit_ids.size())
        throw std::runtime_error("Not enough Init patches for all ids");

    auto core_qubit_ids_itr = core_qubit_ids.begin();
    for (const SparsePatch& p : layout.core_patches())
    {
        Cell cell = slice.place_sparse_patch(p);
        slice.patch_at(cell)->id = *core_qubit_ids_itr++;
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



void advance_slice(DenseSlice& slice, const Layout& layout)
{
    slice.traverse_cells_mut([&](const Cell& c, std::optional<DensePatch>& p) {
        if(!p) return;

        if( p->activity == PatchActivity::Unitary)
        {
            p->activity = PatchActivity::None;
            p->boundaries.top.is_active = false;
            p->boundaries.bottom.is_active = false;
            p->boundaries.left.is_active = false;
            p->boundaries.right.is_active = false;
        }

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
            }
            time_to_magic_state_here = layout.distillation_times()[distillation_region_index];
        }

        distillation_region_index++;
    }
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
        if (!slice.has_patch(p->target))
            return {std::make_unique<std::runtime_error>(lstk::cat("Patch ", p->target, " not on lattice")), {}};

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
            if (slice.get_patch_by_id_mut(p->target).is_active())
                return {std::make_unique<std::runtime_error>(lstk::cat("Patch ", p->target, " is active")), {}};

            slice.get_patch_by_id_mut(p->target).activity = PatchActivity::Unitary;
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
        auto location = find_free_ancilla_location(slice.layout, slice);
        if (!location) return {std::make_unique<std::runtime_error>("Could not allocate ancilla"), {}};

        slice.qubit_patches.push_back(LayoutHelpers::basic_square_patch(*location));
        slice.qubit_patches.back().id = init->target;
        is_cell_free[location->row][location->col] = false;

        return {nullptr, {}};
    }
    else if (const auto* rotation = std::get_if<RotateSingleCellPatch>(&instruction.operation))
    {
        if (!slice.has_patch(rotation->target))
            return {std::make_unique<std::runtime_error>(lstk::cat("Patch ", rotation->target, " not on lattice")), {}};

        const SparsePatch target_patch{slice.get_patch_by_id(rotation->target)};
        if (const auto* target_occupied_cell = std::get_if<SingleCellOccupiedByPatch>(&target_patch.cells))
        {
            std::optional<Cell> free_neighbour;
            compute_free_cells(is_cell_free, slice);
            for (auto neighbour_cell: slice.get_neigbours_within_slice(target_occupied_cell->cell))
                if (is_cell_free[neighbour_cell.row][neighbour_cell.col])
                    free_neighbour = neighbour_cell;

            if (!free_neighbour)
                return {std::make_unique<std::runtime_error>(lstk::cat(
                        "Cannot rotate patch ", rotation->target, ": has no free neighbour")), {}};

            slice.delete_qubit_patch(rotation->target);

            auto stages{LayoutHelpers::single_patch_rotation_a_la_litinski(target_patch, *free_neighbour)};
            slice.routing_regions.push_back(std::move(stages.stage_1));

            return {nullptr, {{BusyRegion{std::move(stages.stage_2), 1, std::move(stages.final_state)}}}};
        }
        else
            return {std::make_unique<std::runtime_error>(lstk::cat(
                    "Cannot rotate patch ", rotation->target, ": is not single cell")), {}};
    }
    else if (auto* mr = std::get_if<MagicStateRequest>(&instruction.operation))
    {
        const auto& d_times = slice.layout.get().distillation_times();
        if (!d_times.size()) throw std::logic_error("No distillation times");

        if (slice.unbound_magic_states.size()>0)
        {
            std::optional<SparsePatch> newly_bound_magic_state = std::move(slice.unbound_magic_states.front());
            slice.unbound_magic_states.pop_front();
            newly_bound_magic_state->id = mr->target;
            newly_bound_magic_state->type = PatchType::Qubit;
            slice.qubit_patches.push_back(*newly_bound_magic_state);
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

            compute_free_cells(is_cell_free, slice);
            bool could_not_find_space_for_patch = false;
            busy_region->state_after_clearing.visit_individual_cells(
                    [&](const SingleCellOccupiedByPatch& occupied_cell){
                        if(!is_cell_free[occupied_cell.cell.row][occupied_cell.cell.col])
                            could_not_find_space_for_patch = true;
                    });

            if(could_not_find_space_for_patch)
                return {std::make_unique<std::runtime_error>(
                        "Could not find space to place patch after rotation"),{std::move(instruction)}};

            slice.qubit_patches.push_back(busy_region->state_after_clearing);
            return {nullptr,{}};
        }
        else
        {
            compute_free_cells(is_cell_free, slice);
            for(const auto& occupied_cell: busy_region->region.cells)
                if(!is_cell_free[occupied_cell.cell.row][occupied_cell.cell.col])
                    return {std::make_unique<std::runtime_error>("Could not find free cell for rotation"),
                            {std::move(instruction)}};

            slice.routing_regions.push_back(busy_region->region);
            return {nullptr,{{BusyRegion{
                    std::move(busy_region->region),
                    busy_region->steps_to_clear-1,
                    std::move(busy_region->state_after_clearing)}}}};
        }

    }

    return {std::make_unique<std::runtime_error>("Unhandled LS instruction in PatchComputation"),{}};
}



void run_through_dense_slices(
        LSInstructionStream&& instruction_stream,
        const Layout& layout,
        Router& router,
        std::optional<std::chrono::seconds> timeout,
        const DenseSliceVisitor& slice_visitor)
{

    try
    {

        DenseSlice slice = first_slice_from_layout(layout, instruction_stream.core_qubits());

        auto start = std::chrono::steady_clock::now();
        size_t ls_op_counter = 0;
        size_t slice_counter = 1;

        std::queue<LSInstruction> future_instructions;

        while (instruction_stream.has_next_instruction() || !future_instructions.empty())
        {
            LSInstruction instruction = [&]()
            {
                if (!future_instructions.empty())
                    return lstk::queue_pop(future_instructions);
                ls_op_counter++;
                return instruction_stream.get_next_instruction();
            }();

            auto application_result = try_apply_instruction(slice, instruction, router);
            if (application_result.maybe_error)
            {
                slice_visitor(slice);
                advance_slice(slice, layout);
                slice_counter++;

                application_result = try_apply_instruction(slice, instruction, router);
                if (application_result.maybe_error)
                    throw std::runtime_error{application_result.maybe_error->what()};

            }

            for (auto&& i: application_result.followup_instructions)
                future_instructions.push(i);

            if (timeout && lstk::since(start)>*timeout)
            {
                auto timeout_str = std::string{"Out of time after "}+std::to_string(timeout->count())+std::string{"s. "}
                        +std::string{"Consumed "}+std::to_string(ls_op_counter)+std::string{" Instructions. "}
                        +std::string{"Generated "}+std::to_string(slice_counter)+std::string{"Slices."};

                throw std::runtime_error{timeout_str};
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "Encountered exception: " << e.what() << std::endl;
        std::cout << "Halting slicing" << std::endl;
    }
}

}