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

    for(auto t : layout.distillation_times())
        slice.time_to_next_magic_state_by_distillation_region.push_back(t);

    return slice;
}


PatchComputation PatchComputation::make(const LogicalLatticeComputation& logical_computation) {
    PatchComputation patch_computation;
    patch_computation.layout = std::make_unique<SimpleLayout>(logical_computation.core_qubits.size());
    patch_computation.slices.push_back(first_slice_from_layout(*patch_computation.layout));
    auto& patches = patch_computation.slices[0].patches;
    auto& ids = logical_computation.core_qubits;
    if(patches.size() < ids.size()){
        throw std::logic_error("Not enough patches for all ids");
    }

    auto patch_itr = patches.begin();
    for (auto id : ids)
    {
        (patch_itr++)->id = id;
    }

    for(const LogicalLatticeOperation& instruction : logical_computation.instructions)
    {

        if (const auto* s = std::get_if<SinglePatchMeasurement>(&instruction.operation))
        {
            Slice& slice = patch_computation.new_slice();
            slice.get_patch_by_id_mut(s->target).activity = PatchActivity::Measurement;
        }
        else if (const auto* p = std::get_if<LogicalPauli>(&instruction.operation))
        {
            Slice& slice = patch_computation.new_slice();
            slice.get_patch_by_id_mut(p->target).activity = PatchActivity::Unitary;
        }
        else if (const auto* m = std::get_if<MultiPatchMeasurement>(&instruction.operation))
        {
            Slice& slice = patch_computation.new_slice();

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
        else
        {
            const auto& mr = std::get<MagicStateRequest>(instruction.operation);

            const auto& d_times = patch_computation.layout->distillation_times();
            if(!d_times.size()) throw std::logic_error("No distillation times");
            size_t max_wait_for_magic_state = *std::max_element(d_times.begin(), d_times.end());

            std::optional<Patch> newly_bound_magic_state;
            for (int i = 0; i<max_wait_for_magic_state; i++)
            {
                auto& slice_with_magic_state = patch_computation.new_slice();
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
                patch_computation.last_slice().patches.push_back(*newly_bound_magic_state);
            }
            else
            {
                throw std::logic_error(
                        absl::StrFormat("Could not get magic state after waiting %d steps", max_wait_for_magic_state));
            }

        }

#if false
        for(const auto p: patch_computation.last_slice().patches)
        {
            auto cell = std::get<SingleCellOccupiedByPatch>(p.cells);
            std::cout << absl::StrFormat("{%d, %d, id=%d} ",cell.cell.row, cell.cell.col, p.id.value_or(99999));
        }
        std::cout<<std::endl;
#endif
    }


    return patch_computation;
}


Slice& PatchComputation::new_slice() {
    slices.push_back(slices.back().advance_slice());
    return slices.back();
}

Slice& PatchComputation::last_slice() {
    return slices.back();
}


}

