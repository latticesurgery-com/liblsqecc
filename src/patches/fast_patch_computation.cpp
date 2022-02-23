#include <lsqecc/patches/patches.hpp>
#include <lsqecc/patches/fast_patch_computation.hpp>
#include <lsqecc/patches/routing.hpp>

#include <absl/strings/str_format.h>

#include <stdexcept>
#include <iterator>
#include <ranges>
#include <iostream>



namespace lsqecc {



Slice first_slice_from_layout(const Layout& layout)
{
    Slice slice{.patches={}, .routing_regions={}, .layout={layout}, .time_to_next_magic_state_by_distillation_region={}};

    for (const Patch& p : layout.core_patches())
        slice.patches.push_back(p);

    size_t distillation_time_offset = 0;
    for(auto t : layout.distillation_times())
        slice.time_to_next_magic_state_by_distillation_region.push_back(t+distillation_time_offset++);

    return slice;
}


Slice advance_slice(const Slice& old_slice, const Layout& layout) {
    Slice new_slice{
        .patches = {},
        .unbound_magic_states = old_slice.unbound_magic_states,
        .layout=old_slice.layout, // TODO should be able to take out
        .time_to_next_magic_state_by_distillation_region={}};

    // Copy patches over
    for (const auto& old_patch : old_slice.patches) {
        // Skip patches that were measured in the previous timestep
        if(old_patch.activity!=PatchActivity::Measurement)
        {
            new_slice.patches.push_back(old_patch);
            auto& new_patch = new_slice.patches.back();

            // Clear Unitary Operator activity
            if (new_patch.activity==PatchActivity::Unitary)
                new_patch.activity = PatchActivity::None;
        }
    }


    // Make magic states appear:
    for (int i = 0; i<old_slice.time_to_next_magic_state_by_distillation_region.size(); ++i)
    {
        new_slice.time_to_next_magic_state_by_distillation_region.push_back(
                old_slice.time_to_next_magic_state_by_distillation_region[i]-1);
        if(new_slice.time_to_next_magic_state_by_distillation_region.back() == 0){

            auto magic_state_cell = new_slice.find_place_for_magic_state(layout.distillation_regions()[i]);
            if(magic_state_cell)
            {
                Patch magic_state_patch = LayoutHelpers::basic_square_patch(*magic_state_cell);
                magic_state_patch.type = PatchType::PreparedState;
                new_slice.unbound_magic_states.push_back(magic_state_patch);
                new_slice.time_to_next_magic_state_by_distillation_region.back() = layout.distillation_times()[i];
            }
#if false
            else
            {
                std::cout<< "Could not find place for magic state produced by distillation region " << i <<std::endl;
            }
#endif
        }
    }

    return new_slice;
}



std::optional<Cell> find_free_ancilla_location(const Layout& layout, const Slice& slice)
{
    for(const Cell& possible_ancilla_location : layout.ancilla_location())
        if(slice.is_cell_free(possible_ancilla_location))
            return possible_ancilla_location;
    return std::nullopt;
}


PatchComputation::PatchComputation(const LogicalLatticeComputation& logical_computation, std::unique_ptr<Layout>&& layout) {
    layout_ = std::move(layout);
    slices_.push_back(first_slice_from_layout(*layout_));

    { // Map initial patches to ids
        auto& init_patches = slices_[0].patches;
        auto& ids = logical_computation.core_qubits;
        if (init_patches.size()<ids.size())
        {
            throw std::logic_error("Not enough Init patches for all ids");
        }

        auto patch_itr = init_patches.begin();
        for (auto id: ids)
        {
            (patch_itr++)->id = id;
        }
    }

    for(const LogicalLatticeOperation& instruction : logical_computation.instructions)
    {

        if (const auto* s = std::get_if<SinglePatchMeasurement>(&instruction.operation))
        {
            Slice& slice = new_slice();
            slice.get_patch_by_id_mut(s->target).activity = PatchActivity::Measurement;
        }
        else if (const auto* p = std::get_if<SingleQubitOp>(&instruction.operation))
        {
            if(p->op == SingleQubitOp::Operator::S)
            {
                Slice& pre_slice = new_slice();
                auto path = do_s_gate_routing(pre_slice, p->target);
                if(!path)
                    throw std::logic_error(absl::StrFormat("Couldn't find room to do an S gate measurement on patch %d", p->target));
                pre_slice.routing_regions.push_back(*path);
            }
            Slice& slice = new_slice();
            slice.get_patch_by_id_mut(p->target).activity = PatchActivity::Unitary;
        }
        else if (const auto* m = std::get_if<MultiPatchMeasurement>(&instruction.operation))
        {
            Slice& slice = new_slice();

            if(m->observable.size()!=2)
                throw std::logic_error("Multi patch measurement only supports 2 patches currently");
            auto pairs = m->observable.begin();
            const auto& [source_id, source_op] = *pairs++;
            const auto& [target_id, target_op] = *pairs;

            auto path = graph_search_route_ancilla(slice, source_id, source_op, target_id, target_op);

            if(path)
                slice.routing_regions.push_back(*path);
            else
                throw std::logic_error(absl::StrFormat("Couldn't find a path from %d to %d", source_id, target_id));
        }
        else if (const auto* i = std::get_if<PatchInit>(&instruction.operation))
        {
            Slice& slice = new_slice();
            auto location = find_free_ancilla_location(*layout_, slice);
            if(!location) throw std::logic_error(absl::StrFormat("Could not allocate ancilla"));

            slice.patches.push_back(LayoutHelpers::basic_square_patch(*location));
            slice.patches.back().id = i->target;
        }
        else
        {
            const auto& mr = std::get<MagicStateRequest>(instruction.operation);

            const auto& d_times = layout_->distillation_times();
            if(!d_times.size()) throw std::logic_error("No distillation times");
            size_t max_wait_for_magic_state = *std::max_element(d_times.begin(), d_times.end());

            std::optional<Patch> newly_bound_magic_state;
            for (int i = 0; i<max_wait_for_magic_state; i++)
            {
                auto& slice_with_magic_state = new_slice();
                if(slice_with_magic_state.unbound_magic_states.size()>1)
                {
                    newly_bound_magic_state = slice_with_magic_state.unbound_magic_states.front();
                    slice_with_magic_state.unbound_magic_states.pop_front();
                    break;
                }
            }

            if(newly_bound_magic_state)
            {
                newly_bound_magic_state->id = mr.target;
                newly_bound_magic_state->type = PatchType::Qubit;
                last_slice().patches.push_back(*newly_bound_magic_state);
            }
            else
            {
                throw std::logic_error(
                        absl::StrFormat("Could not get magic state after waiting %d steps", max_wait_for_magic_state));
            }

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
}


Slice& PatchComputation::new_slice() {
    slices_.push_back(advance_slice(slices_.back(), *layout_));
    return slices_.back();
}

Slice& PatchComputation::last_slice() {
    return slices_.back();
}


}

