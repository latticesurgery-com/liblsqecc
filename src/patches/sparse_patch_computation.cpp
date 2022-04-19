#include <lsqecc/patches/patches.hpp>
#include <lsqecc/patches/sparse_patch_computation.hpp>
#include <lsqecc/layout/router.hpp>

#include <lstk/lstk.hpp>

#include <stdexcept>
#include <iterator>
#include <ranges>
#include <iostream>
#include <sstream>



namespace lsqecc {


std::optional<Cell> SparsePatchComputation::find_place_for_magic_state(size_t distillation_region_idx) const
{
    compute_free_cells(is_cell_free_, slice_store_.last_slice_const());
    for(const auto& cell: layout_->distilled_state_locations(distillation_region_idx))
        if(is_cell_free_[cell.row][cell.col])
            return cell;

    return std::nullopt;
}


SparseSlice first_slice_from_layout(const Layout& layout, const tsl::ordered_set<PatchId>& core_qubit_ids)
{
    SparseSlice slice{{}, {},{}, {layout},{}};


    for (const SparsePatch& p : layout.core_patches())
        slice.qubit_patches.push_back(p);

    size_t distillation_time_offset = 0;
    for(auto t : layout.distillation_times())
        slice.time_to_next_magic_state_by_distillation_region.push_back(t+distillation_time_offset++);


    // Map initial patches to ids
    if (slice.qubit_patches.size()<core_qubit_ids.size())
        throw std::logic_error("Not enough Init patches for all ids");


    auto patch_itr = slice.qubit_patches.begin();
    for (const auto& id: core_qubit_ids)
    {
        (patch_itr++)->id = id;
    }

    return slice;
}


SparseSlice& SparsePatchComputation::make_new_slice()
{

    slice_visitor_(slice_store_.last_slice());

    // This is an expensive copy. To avoid doing it twice we do in in place on the heap
    // TODO avoid this copy by doing a move and updating things

    SparseSlice new_slice{
        {},
        {},
        slice_store_.last_slice().unbound_magic_states,
        *layout_, // TODO should be able to take out
        {}};

    const SparseSlice& old_slice = slice_store_.last_slice();

    // Copy patches over
    for (const auto& old_patch : old_slice.qubit_patches)
    {
        // Skip patches that were measured in the previous timestep
        if(old_patch.activity!=PatchActivity::Measurement)
        {
            new_slice.qubit_patches.push_back(old_patch);
            auto& new_patch = new_slice.qubit_patches.back();

            // Clear Unitary Operator activity
            if (new_patch.activity==PatchActivity::Unitary)
                new_patch.activity = PatchActivity::None;

            new_patch.visit_individual_cells_mut([](SingleCellOccupiedByPatch& c)
            {
               c.top.is_active = false;
               c.bottom.is_active = false;
               c.left.is_active = false;
               c.right.is_active = false;
            });
        }
        else
        {
            for (const Cell &cell : old_patch.get_cells())
                is_cell_free_[cell.row][cell.col] = true;
        }
    }


    // Make magic states appear:
    for (size_t i = 0; i<old_slice.time_to_next_magic_state_by_distillation_region.size(); ++i)
    {
        new_slice.time_to_next_magic_state_by_distillation_region.push_back(
                old_slice.time_to_next_magic_state_by_distillation_region[i]-1);
        if(new_slice.time_to_next_magic_state_by_distillation_region.back() == 0){

            auto magic_state_cell = find_place_for_magic_state(i);
            if(magic_state_cell)
            {
                SparsePatch magic_state_patch = LayoutHelpers::basic_square_patch(*magic_state_cell);
                magic_state_patch.type = PatchType::PreparedState;
                new_slice.unbound_magic_states.push_back(magic_state_patch);
                is_cell_free_[magic_state_cell->row][magic_state_cell->col] = false;
            }
#if false
            else
            {
                std::cout<< "Could not find place for magic state produced by distillation region " << i <<std::endl;
            }
#endif
            new_slice.time_to_next_magic_state_by_distillation_region.back() = layout_->distillation_times()[i];
        }
    }
    slice_store_.accept_new_slice(std::move(new_slice));
    return slice_store_.last_slice();
}



std::optional<Cell> find_free_ancilla_location(const Layout& layout, const SparseSlice& slice)
{
    for(const Cell& possible_ancilla_location : layout.ancilla_location())
        if(slice.is_cell_free(possible_ancilla_location))
            return possible_ancilla_location;
    return std::nullopt;
}


void stitch_boundaries(SparseSlice& slice, PatchId source, PatchId target, RoutingRegion& routing_region)
{
    auto& source_patch = slice.get_single_cell_occupied_by_patch_by_id_mut(source);
    auto& target_patch = slice.get_single_cell_occupied_by_patch_by_id_mut(target);

    if (routing_region.cells.empty())
    {
        source_patch.get_mut_boundary_with(target_patch.cell)->get().is_active=true;
        target_patch.get_mut_boundary_with(source_patch.cell)->get().is_active=true;
    }
    else
    {
#if false
        std::cout<<target_patch.cell<<"->";
        for(auto c : routing_region.cells)
            std::cout<<c.cell<<"->";
        std::cout << source_patch.cell<< std::endl;
#endif
        source_patch.get_mut_boundary_with(routing_region.cells.back().cell)->get().is_active=true;
        target_patch.get_mut_boundary_with(routing_region.cells.front().cell)->get().is_active=true;
    }
}

// TODO this file ha several helpers like this one, that could be moved to a separate file
/*
 * Returns true iff merge was successful
 */
bool merge_patches(
        SparseSlice& slice,
        Router& router,
        PatchId source,
        PauliOperator source_op,
        PatchId target,
        PauliOperator target_op)
{

    if(slice.get_patch_by_id(source).is_active() || slice.get_patch_by_id(target).is_active())
        return false;

    auto routing_region = router.find_routing_ancilla(slice, source, source_op, target, target_op);
    if(!routing_region)
        return false;


    stitch_boundaries(slice, source, target, *routing_region);
    if(!routing_region->cells.empty())
        slice.routing_regions.push_back(std::move(*routing_region));

    return true;
}



// TODO replace with a variant
struct InstructionApplicationResult
{
    std::unique_ptr<std::exception> maybe_error;
    std::vector<LSInstruction> followup_instructions;
};


InstructionApplicationResult try_apply_instruction(
        SparseSlice& slice,
        LSInstruction instruction,
        Router& router,
        FreeCellCache is_cell_free)
{
    if (const auto* s = std::get_if<SinglePatchMeasurement>(&instruction.operation))
    {
        if (!slice.has_patch(s->target))
            return {std::make_unique<std::runtime_error>(lstk::cat("Patch ", s->target, " not on lattice")), {}};

        auto& target_patch = slice.get_patch_by_id_mut(s->target);
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


void SparsePatchComputation::make_slices(
        LSInstructionStream&& instruction_stream,
        std::optional<std::chrono::seconds> timeout)
{
    slice_store_.accept_new_slice(first_slice_from_layout(*layout_, instruction_stream.core_qubits()));
    ls_instructions_count_++; // The declare qubits instruction

    // Add a blank slice for the initial visualization of the layout
    make_new_slice();

    auto start = std::chrono::steady_clock::now();
    size_t ls_op_counter = 0;

    std::queue<LSInstruction> future_instructions;

    while (instruction_stream.has_next_instruction() || !future_instructions.empty())
    {
        LSInstruction instruction{[&](){
            if(!future_instructions.empty())
                return lstk::queue_pop(future_instructions);
            ls_instructions_count_++;
            return instruction_stream.get_next_instruction();
        }()};

        auto application_result = try_apply_instruction(slice_store_.last_slice(), instruction, *router_, is_cell_free_);
        if(application_result.maybe_error)
        {
            make_new_slice();
            application_result = try_apply_instruction(slice_store_.last_slice(), instruction, *router_, is_cell_free_);
            if(application_result.maybe_error)
                throw std::runtime_error{application_result.maybe_error->what()};

        }

        for(auto&& i : application_result.followup_instructions)
            future_instructions.push(i);

        ls_op_counter++;
        if(timeout && lstk::since(start) > *timeout)
        {
            auto timeout_str = std::string{"Out of time after "}+std::to_string(timeout->count())+std::string{"s. "}
                    + std::string{"Consumed "} + std::to_string(ls_op_counter) + std::string{" Instructions. "}
                    + std::string{"Generated "} + std::to_string(slice_store_.slice_count()) + std::string{"Slices."};

            throw std::runtime_error{timeout_str};
        }

#if false
        for(const auto p: last_slice().patches)
        {
            auto cell = std::get<SingleCellOccupiedByPatch>(p.cells);
            std::cout << absl::StrFormat("{%d, %d, id=%d} ",cell.cell.row, cell.cell.col, p.id.value_or(99999));
        }
        std::cout<<std::endl;
#endif
    }

    slice_visitor_(slice_store_.last_slice());

}


void compute_free_cells(FreeCellCache& is_cell_free, const SparseSlice& slice)
{
    std::for_each(is_cell_free.begin(), is_cell_free.end(), [](std::vector<lstk::bool8>& row){
        std::fill(row.begin(), row.end(), true);
    });

    auto mark_as_not_free = [&is_cell_free](const SingleCellOccupiedByPatch& occupied_cell){
        is_cell_free[occupied_cell.cell.row][occupied_cell.cell.col] = false;};

    for(const SparsePatch& p: slice.qubit_patches)
        p.visit_individual_cells(mark_as_not_free);

    for(const auto& rr : slice.routing_regions)
        for(const auto& occupied_cell : rr.cells)
            mark_as_not_free(occupied_cell);

}


SparsePatchComputation::SparsePatchComputation(
        LSInstructionStream&& instruction_stream,
        std::unique_ptr<Layout>&& layout,
        std::unique_ptr<Router>&& router,
        std::optional<std::chrono::seconds> timeout,
        SliceVisitorFunction slice_visitor)
        :slice_store_(*layout), slice_visitor_(slice_visitor)
        {
    layout_ = std::move(layout);
    router_ = std::move(router);

    for(Cell::CoordinateType row = 0; row<=layout_->furthest_cell().row; row++ )
        is_cell_free_.push_back(std::vector<lstk::bool8>(static_cast<size_t>(layout_->furthest_cell().col+1), false));

    try
    {
        make_slices(std::move(instruction_stream), timeout);
    }
    catch (const std::exception& e)
    {
        std::cout << "Encountered exception: " << e.what() << std::endl;
        std::cout << "Halting slicing" << std::endl;
    }

}


void SliceStore::accept_new_slice(SparseSlice&& slice)
{
    second_last_slice_ = std::move(last_slice_);
    last_slice_ = std::move(slice);
    slice_count_++;
}


SliceStore::SliceStore(const Layout& layout)
    :last_slice_(SparseSlice::make_blank_slice(layout)), second_last_slice_(SparseSlice::make_blank_slice(layout))
{}

}

