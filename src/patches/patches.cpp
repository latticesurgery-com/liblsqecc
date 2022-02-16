#include <lsqecc/patches/patches.hpp>

namespace lsqecc {

PatchId global_patch_id_counter = 0;

PatchId make_new_patch_id(){
    return global_patch_id_counter++;
}

std::vector<Cell> Patch::get_cells() const
{
    if(auto single_cell_patch = std::get_if<SingleCellOccupiedByPatch>(&cells))
    {
        return {single_cell_patch->cell};
    }
    else
    {
        const auto& multi_cell_patch = std::get<MultipleCellsOccupiedByPatch>(cells);
        return std::vector<Cell>{multi_cell_patch.sub_cells.begin(), multi_cell_patch.sub_cells.end()};
    }
}

}