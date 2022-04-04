#ifndef LSQECC_LAYOUT_HPP
#define LSQECC_LAYOUT_HPP


#include <lsqecc/patches/patches.hpp>
#include <lstk/lstk.hpp>

#include <tuple>

namespace lsqecc {

using SurfaceCodeTimestep = uint32_t;


struct Layout {

    virtual const std::vector<Patch>& core_patches() const = 0;
    virtual Cell furthest_cell() const = 0;
    virtual const std::vector<MultipleCellsOccupiedByPatch>& distillation_regions() const = 0;
    virtual const std::vector<Cell>& distilled_state_locations(size_t distillation_region_idx) const = 0;
    virtual const std::vector<SurfaceCodeTimestep>& distillation_times() const = 0;
    virtual const std::vector<Cell>& ancilla_location() const = 0;

    template<class F> void for_each_cell(F f) const;

    virtual ~Layout() = default;
};


namespace LayoutHelpers{
    Patch basic_square_patch(Cell placement);
    SingleCellOccupiedByPatch make_distillation_region_cell(Cell placement);


    struct SinglePatchRotationALaLitinskiStages
    {
        RoutingRegion stage_1;
        RoutingRegion stage_2;
        Patch final_state;
    };
    SinglePatchRotationALaLitinskiStages single_patch_rotation_a_la_litinski(
            const Patch& target_patch, const Cell& free_neighbour);

}


class SimpleLayout : public Layout {
public:

    explicit SimpleLayout(size_t num_qubits)
    : num_qubits_(num_qubits), distillation_times_(), core_patches_(), distillation_regions_(2) {
        using namespace LayoutHelpers;

        core_patches_.reserve(num_qubits_);
        for(size_t i = 0; i<num_qubits_; i++) {
            core_patches_.push_back(basic_square_patch({
                    .row=0,
                    .col=2*static_cast<Cell::CoordinateType>(i)
            }));
        }

        distillation_regions_ = {
                MultipleCellsOccupiedByPatch{.sub_cells={
                        make_distillation_region_cell(Cell{3, 0}), make_distillation_region_cell(Cell{3, 1}),
                        make_distillation_region_cell(Cell{3, 2}),
                        make_distillation_region_cell(Cell{4, 0}), make_distillation_region_cell(Cell{4, 1}),
                        make_distillation_region_cell(Cell{4, 2}),
                        make_distillation_region_cell(Cell{5, 0}), make_distillation_region_cell(Cell{5, 1}),
                        make_distillation_region_cell(Cell{5, 2}),
                }},
                MultipleCellsOccupiedByPatch{.sub_cells={
                        make_distillation_region_cell(Cell{3, 4}), make_distillation_region_cell(Cell{3, 5}),
                        make_distillation_region_cell(Cell{3, 6}),
                        make_distillation_region_cell(Cell{4, 4}), make_distillation_region_cell(Cell{4, 5}),
                        make_distillation_region_cell(Cell{4, 6}),
                        make_distillation_region_cell(Cell{5, 4}), make_distillation_region_cell(Cell{5, 5}),
                        make_distillation_region_cell(Cell{5, 6}),
                }},
        };

        distilled_state_locations_ = {{Cell{2,0},Cell{2,1},Cell{2,2}},
                                      {Cell{2,3},Cell{2,4},Cell{2,5}},};

        ancilla_locations_ = {Cell{1,7}};

        for(const auto& r: distillation_regions_)
        {
            distillation_times_.push_back(5);
            LSTK_UNUSED(r);
        }
    }

    const std::vector<Patch>& core_patches() const override
    {
        return core_patches_;
    }

    Cell furthest_cell() const override {
        return Cell{5,7};
    }

    const std::vector<SurfaceCodeTimestep>& distillation_times() const override {
        return distillation_times_;
    }

    const std::vector<MultipleCellsOccupiedByPatch>& distillation_regions() const override
    {
        return distillation_regions_;
    }

    const std::vector<Cell>& distilled_state_locations(size_t distillation_region_idx) const override
    {
        return distilled_state_locations_[distillation_region_idx];
    }

    const std::vector<Cell>& ancilla_location() const override
    {
        return ancilla_locations_;
    }

private:
    size_t num_qubits_;
    std::vector<SurfaceCodeTimestep> distillation_times_;
    std::vector<Patch> core_patches_;
    std::vector<MultipleCellsOccupiedByPatch> distillation_regions_;
    std::vector<Cell> ancilla_locations_;
    std::vector<std::vector<Cell>> distilled_state_locations_;

};


template<class F> void Layout::for_each_cell(F f) const
{
    auto [max_row, max_col] = furthest_cell();
    for(Cell::CoordinateType row = 0; row<=max_row; row++ )
        for(Cell::CoordinateType col = 0; col<=max_col; col++)
            f(Cell{row,col});
}


}


#endif //LSQECC_LAYOUT_HPP
