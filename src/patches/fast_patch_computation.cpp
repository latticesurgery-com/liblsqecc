#include <lsqecc/patches/patches.hpp>
#include <lsqecc/patches/fast_patch_computation.hpp>


#include <stdexcept>
#include <iterator>
#include <ranges>



namespace lsqecc {


class SimpleLayout : public Layout {
public:
    explicit SimpleLayout(size_t num_qubits) : num_qubits_(num_qubits) {}

    const std::vector<Patch> core_patches() const override
    {
        std::vector<Patch> core;
        core.reserve(num_qubits_);
        for(size_t i: std::views::iota(static_cast<size_t>(0),num_qubits_))
        {
            core.push_back(basic_square_patch({
                    .row=0,
                    .col=2*static_cast<Cell::CoordinateType>(i)
            }));
        }
        return core;
    }

    std::vector<Cell> magic_state_queue_locations() const override {
        std::vector<Cell> magic_state_queue;
        magic_state_queue.reserve(num_qubits_);
        for(size_t i: std::views::iota(static_cast<size_t>(0),num_qubits_))
        {
            magic_state_queue.push_back({
                    .row=0,
                    .col=2*static_cast<Cell::CoordinateType>(i)
            });
        }
        return magic_state_queue;
    }
    std::vector<Cell> distillery_locations()  const override
    {
        return {};
    };

private:
    static Patch basic_square_patch(Cell placement){
        return Patch{
                .cells=SingleCellOccupiedByPatch{
                        .top={BoundaryType::Rough,false},
                        .bottom={BoundaryType::Rough,false},
                        .left={BoundaryType::Smooth,false},
                        .right={BoundaryType::Smooth,false},
                        .cell=placement
                },
                .type=PatchType::Qubit,
                .id=std::nullopt,
        };
    }

private:
    size_t num_qubits_;

};



Slice first_slice_from_layout(const Layout& layout)
{
    Slice slice{.distance_dependant_timesteps=1, .patches={}};

    for (const Patch& p : layout.core_patches())
        slice.patches.push_back(p);

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

        // Resume here by finishing the end to end pipeline with these two operations:
        // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

        if (const auto* s = std::get_if<SinglePatchMeasurement>(&instruction.operation))
        {
            slice.get_patch_by_id(s->target).activity = PatchActivity::Measurement;
        }
        else if (const auto* p = std::get_if<LogicalPauli>(&instruction.operation))
        {
            slice.get_patch_by_id(p->target).activity = PatchActivity::Unitary;
        }
        else if (const auto* m = std::get_if<MultiPatchMeasurement>(&instruction.operation))
        {

        }
        else
        {
            const auto& mr = std::get<MagicStateRequest>(instruction.operation);
        }
    }


    return patch_computation;
}


size_t PatchComputation::num_slices() const
{
    return slices.size();
}


const Slice& PatchComputation::slice(size_t idx) const
{
    return slices.at(idx);
}

const Slice& PatchComputation::last_slice() const {
    return slice(num_slices()-1);
}

Slice& PatchComputation::new_slice() {
    slices.emplace_back(last_slice().make_copy_with_cleared_activity());
    return slices.back();
}


}

