#include <lsqecc/patches/patches.hpp>
#include <lsqecc/patches/fast_patch_computation.hpp>
#include <lsqecc/patches/routing.hpp>


#include <stdexcept>
#include <iterator>
#include <ranges>




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
        Slice& slice = patch_computation.new_slice();

        if (const auto* s = std::get_if<SinglePatchMeasurement>(&instruction.operation))
        {
            slice.get_patch_by_id_mut(s->target).activity = PatchActivity::Measurement;
        }
        else if (const auto* p = std::get_if<LogicalPauli>(&instruction.operation))
        {
            slice.get_patch_by_id_mut(p->target).activity = PatchActivity::Unitary;
        }
        else if (const auto* m = std::get_if<MultiPatchMeasurement>(&instruction.operation))
        {
            if(m->observable.size()!=2)
                throw std::logic_error("Multi patch measurement only supports 2 patches currently");
            auto pairs = m->observable.begin();
            const auto& [source_id, source_op] = *pairs++;
            const auto& [target_id, target_op] = *pairs;
            slice.routing_regions.push_back(
                    graph_search_route_ancilla(slice, source_id, source_op, target_id, target_op)
            );
        }
        else
        {
            const auto& mr = std::get<MagicStateRequest>(instruction.operation);
        }
    }


    return patch_computation;
}


Slice& PatchComputation::new_slice() {
    slices.emplace_back(slices.back().advance_slice());
    return slices.back();
}


}

