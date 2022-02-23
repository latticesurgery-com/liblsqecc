#include <lsqecc/patches/slice.hpp>
#include <iterator>
#include <algorithm>
#include <iostream>

namespace lsqecc{


std::optional<Cell> Slice::find_place_for_magic_state(const MultipleCellsOccupiedByPatch& distillation_region) const
{
    const auto& cells = distillation_region.sub_cells;
    for(const auto& cell: cells)
        for(const auto& neighbour: get_neigbours_within_slice(cell.cell))
            if(is_cell_free(neighbour))
                return neighbour;

    return std::nullopt;
}



Patch& Slice::get_patch_by_id_mut(PatchId id) {
    for(auto& p: patches)
    {
        if(p.id == id)
        {
            return p;
        }
    }
    throw std::logic_error(std::string{"No patch for id: "+std::to_string(id)});
}

const Patch& Slice::get_patch_by_id(PatchId id) const {
    for(auto& p: patches)
    {
        if(p.id == id)
        {
            return p;
        }
    }
    throw std::logic_error(std::string{"No patch for id: "+std::to_string(id)});
}

Cell Slice::get_furthest_cell() const
{
    Cell ret{layout.min_furthest_cell()};
    for(const Patch& p: patches)
    {
        for (const Cell& c: p.get_cells())
        {
            ret.col = std::max(ret.col, c.col);
            ret.row = std::max(ret.row, c.row);
        }
    }
    return ret;
}


std::optional<std::reference_wrapper<const Patch>> Slice::get_patch_on_cell(const Cell& cell) const
{
    for(const Patch& p: patches)
        for (const Cell& c: p.get_cells())
            if(c==cell)
                return p;

    for(const Patch& p: unbound_magic_states)
        for (const Cell& c: p.get_cells())
            if(c==cell)
                return p;

    return std::nullopt;
}

bool Slice::is_cell_free(const Cell& cell) const
{
    for(const auto& distillation_region: layout.distillation_regions())
        for(const auto& distillation_cell: distillation_region.sub_cells)
            if(distillation_cell.cell==cell)
                return false;

    return !Slice::get_patch_on_cell(cell);
}

std::vector<Cell> Slice::get_neigbours_within_slice(const Cell& cell) const
{
    return cell.get_neigbours_within_bounding_box_inclusive({0,0},get_furthest_cell());
}

}
