#ifndef LSQECC_LAYOUT_HPP
#define LSQECC_LAYOUT_HPP


#include <lsqecc/patches/patches.hpp>

namespace lsqecc {

using SurfaceCodeTimestep = uint32_t;


struct Layout {

    virtual const std::vector<Patch>& core_patches() const = 0;
    virtual Cell min_furthest_cell() const = 0;
    virtual const std::vector<MultipleCellsOccupiedByPatch>& distillation_regions() const = 0;
    virtual const std::vector<SurfaceCodeTimestep>& distillation_times() const = 0;

    virtual ~Layout(){};
};


namespace LayoutHelpers{
    Patch basic_square_patch(Cell placement);
    SingleCellOccupiedByPatch make_distillation_region_cell(Cell placement);
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

        for(const auto& r: distillation_regions_)
            distillation_times_.push_back(5);
    }

    const std::vector<Patch>& core_patches() const override
    {
        return core_patches_;
    }

    Cell min_furthest_cell() const override {
        return Cell{4,6}; // Could extract from distillation region;
    }

    const std::vector<SurfaceCodeTimestep>& distillation_times() const override {
        return distillation_times_;
    }

    const std::vector<MultipleCellsOccupiedByPatch>& distillation_regions() const override
    {
        return distillation_regions_;
    }

private:
    size_t num_qubits_;
    std::vector<SurfaceCodeTimestep> distillation_times_;
    std::vector<Patch> core_patches_;
    std::vector<MultipleCellsOccupiedByPatch> distillation_regions_;

};


}


#endif //LSQECC_LAYOUT_HPP
