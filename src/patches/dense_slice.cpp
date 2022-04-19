#include <lsqecc/patches/dense_slice.hpp>

namespace lsqecc
{


void DenseSlice::traverse_cells_mut(const DenseSlice::CellTraversalFunctor& f)
{
    Cell c {0,0};
    for(auto& row: cells)
    {
        for (std::optional<DensePatch>& patch: row)
        {
            c.col++;
            f(c,patch);
        }
        c.row = 0;
        c.col++;
    }

}

void DenseSlice::traverse_cells(const CellTraversalConstFunctor& f) const
{
    const_cast<DenseSlice*>(this)->traverse_cells_mut(f);
}



std::optional<std::reference_wrapper<DensePatch>> DenseSlice::get_patch_by_id_mut(PatchId id)
{
    std::optional<std::reference_wrapper<DensePatch>> ret;
    traverse_cells_mut([&](const Cell& c, std::optional<DensePatch>& p) {
        if(p && p->id == id)
            ret = std::ref(*p);
    });
    return ret;
}


std::optional<std::reference_wrapper<const DensePatch>> DenseSlice::get_patch_by_id(PatchId id) const
{
    return const_cast<DenseSlice*>(this)->get_patch_by_id_mut(id);
}

std::optional<std::reference_wrapper<DensePatch>> DenseSlice::get_patch_on_cell_mut(const Cell& cell)
{
    std::optional<std::reference_wrapper<DensePatch>> ret;
    traverse_cells_mut([&](const Cell& c, std::optional<DensePatch>& p) {
        if(c == cell && p) ret = std::ref(*p);
    });
    return ret;
}

std::optional<std::reference_wrapper<const DensePatch>> DenseSlice::get_patch_on_cell(const Cell& cell) const
{
    return std::cref(const_cast<DenseSlice*>(this)->get_patch_on_cell_mut(cell)->get());
}

void DenseSlice::delete_qubit_patch(PatchId id)
{
    traverse_cells_mut([&](const Cell& c, std::optional<DensePatch>& p) {
        if(p && p->id == id) p = std::nullopt;
    });
}

std::vector<Cell> DenseSlice::get_neigbours_within_slice(const Cell& cell) const
{
    return cell.get_neigbours_within_bounding_box_inclusive({0,0}, furthest_cell());
}

DenseSlice DenseSlice::make_blank_slice(const Layout& layout)
{
    return DenseSlice{
        .cells=std::vector<RowStore>(
            layout.furthest_cell().col+1,
            RowStore(layout.furthest_cell().row+1, std::nullopt))
    };
}

bool DenseSlice::is_cell_free(const Cell& cell) const
{
    return get_patch_on_cell(cell).has_value();
}


}