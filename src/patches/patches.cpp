#include <lsqecc/patches/patches.hpp>

namespace lsqecc {



BoundaryType boundary_for_operator(PauliOperator op){
    switch (op)
    {
    case PauliOperator::X: return BoundaryType::Rough;
    case PauliOperator::Z: return BoundaryType::Smooth;
    case PauliOperator::I: throw std::logic_error("No boundary for I operator");
    case PauliOperator::Y: throw std::logic_error("No boundary for Y operator");
    }
}

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
        std::vector<Cell> out_cells{multi_cell_patch.sub_cells.size()};
        for (const auto& cell: multi_cell_patch.sub_cells)
        {
            out_cells.push_back(cell.cell);
        }
        return out_cells;
    }
}


const Cell& Patch::get_a_cell() const
{
    if(auto single_cell_patch = std::get_if<SingleCellOccupiedByPatch>(&cells))
    {
        return single_cell_patch->cell;
    }
    else
    {
        const auto& multi_cell_patch = std::get<MultipleCellsOccupiedByPatch>(cells);
        return multi_cell_patch.sub_cells.front().cell;
    }
}

std::vector<Cell> Cell::get_neigbours() const
{
    return {Cell{row-1, col},
            Cell{row+1, col},
            Cell{row, col-1},
            Cell{row, col+1}};
}

std::optional<Boundary> SingleCellOccupiedByPatch::get_boundary_with(const Cell& neighbour) const
{
    if(neighbour == Cell{cell.row-1, cell.col})   return top;
    if(neighbour == Cell{cell.row+1, cell.col})   return bottom;
    if(neighbour == Cell{cell.row,   cell.col-1}) return left;
    if(neighbour == Cell{cell.row,   cell.col+1}) return right;

    return std::nullopt;
}

std::optional<std::reference_wrapper<Boundary>> SingleCellOccupiedByPatch::get_mut_boundary_with(const Cell& neighbour)
{
    if(neighbour == Cell{cell.row-1, cell.col})   return top;
    if(neighbour == Cell{cell.row+1, cell.col})   return bottom;
    if(neighbour == Cell{cell.row,   cell.col-1}) return left;
    if(neighbour == Cell{cell.row,   cell.col+1}) return right;

    return std::nullopt;
}


}