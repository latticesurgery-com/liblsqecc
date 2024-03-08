#ifndef LSQECC_LAYOUT_HPP
#define LSQECC_LAYOUT_HPP


#include <lsqecc/patches/patches.hpp>
#include <lstk/lstk.hpp>

#include <tuple>

namespace lsqecc {

using SurfaceCodeTimestep = uint32_t;
using DistillationTimeMap = std::vector<SurfaceCodeTimestep>;


struct Layout {

    virtual const std::vector<SparsePatch>& core_patches() const = 0;
    virtual Cell furthest_cell() const = 0;
    virtual const std::vector<MultipleCellsOccupiedByPatch>& distillation_regions() const = 0;
    virtual const std::vector<Cell>& distilled_state_locations(size_t distillation_region_idx) const = 0;
    virtual const DistillationTimeMap& distillation_times() const = 0;
    virtual const std::vector<Cell>& ancilla_location() const = 0;
    virtual const std::vector<Cell>& dead_location() const = 0;
    virtual const std::vector<Cell>& predistilled_y_states() const = 0;
    virtual const bool magic_states_reserved() const = 0;

    template<class F> void for_each_cell(F f) const;

    virtual ~Layout() = default;
};


struct DistillationOptions{
    bool staggered = true;
    size_t distillation_time = 10;
};

namespace LayoutHelpers{
    SparsePatch basic_square_patch(Cell placement, std::optional<PatchId> id = std::nullopt, std::optional<std::string> debug_label = std::nullopt);
    SingleCellOccupiedByPatch make_distillation_region_cell(Cell placement);
}


template<class F> void Layout::for_each_cell(F f) const
{
    auto [max_row, max_col] = furthest_cell();
    for(Cell::CoordinateType row = 0; row<=max_row; row++ )
        for(Cell::CoordinateType col = 0; col<=max_col; col++)
            f(Cell{row,col});
}


}


#endif //LSQECC_LAYOUT_HPP
