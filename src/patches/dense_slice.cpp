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



std::optional<std::reference_wrapper<DensePatch>> DenseSlice::get_patch_by_id(PatchId id)
{
    std::optional<std::reference_wrapper<DensePatch>> ret;
    traverse_cells_mut([&](const Cell& c, std::optional<DensePatch>& p) {
        if(p && p->id == id)
            ret = std::ref(*p);
    });
    return ret;
}


std::optional<std::reference_wrapper<DensePatch const>> DenseSlice::get_patch_by_id(PatchId id) const
{
    auto maybe_patch = const_cast<DenseSlice*>(this)->get_patch_by_id(id);
    return maybe_patch ? std::make_optional(std::cref(maybe_patch->get())) : std::nullopt;
}

std::optional<DensePatch>& DenseSlice::patch_at(const Cell& cell)
{
    using MaybeDensePatch = std::optional<DensePatch>;
    std::optional<std::reference_wrapper<MaybeDensePatch>>ret;
    traverse_cells_mut([&](const Cell& c, std::optional<DensePatch>& p) {
        if(c == cell) ret = std::ref(p);
    });

    if(!ret) throw std::logic_error(lstk::cat("Cell (", cell.row, ", ", cell.col, ") out of bounds"));
    return ret->get();
}

const std::optional<DensePatch>& DenseSlice::patch_at(const Cell& cell) const
{
    return const_cast<DenseSlice*>(this)->patch_at(cell);
}

void DenseSlice::delete_patch_by_id(PatchId id)
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
    return patch_at(cell).has_value();
}


}