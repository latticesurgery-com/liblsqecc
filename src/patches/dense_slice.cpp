#include <lsqecc/patches/dense_slice.hpp>

namespace lsqecc
{




const Layout& DenseSlice::get_layout() const
{
    return layout.get();
}


void DenseSlice::traverse_cells_mut(const DenseSlice::CellTraversalFunctor& f)
{
    Cell c {0,0};
    for(std::vector<std::optional<DensePatch>>& row: cells)
    {
        for (std::optional<DensePatch>& patch: row)
        {
            f(c,patch);
            c.col++;
        }
        c.row++;
        c.col = 0;
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

std::optional<SparsePatch> DenseSlice::get_sparse_patch_by_id(lsqecc::PatchId id) const
{
    auto dense_patch = get_patch_by_id(id);
    if(!dense_patch) return std::nullopt;
    const DensePatch& dense_patch_ref = dense_patch->get();

    return SparsePatch{
            {static_cast<Patch>(dense_patch_ref)},
            SingleCellOccupiedByPatch{
                    {dense_patch_ref.boundaries},
                    get_cell_by_id(id).value()
            },
    };
}

std::optional<Cell> DenseSlice::get_cell_by_id(PatchId id) const
{
    std::optional<Cell> ret;
    traverse_cells([&](const Cell& c, const std::optional<DensePatch>& p) {
        if(p && p->id == id)
            ret = c;
    });
    return ret;
}

std::optional<DensePatch>& DenseSlice::patch_at(const Cell& cell)
{
    return cells.at(cell.row).at(cell.col);
}

const std::optional<DensePatch>& DenseSlice::patch_at(const Cell& cell) const
{
    return cells.at(cell.row).at(cell.col);
}

void DenseSlice::delete_patch_by_id(PatchId id)
{
    traverse_cells_mut([&](const Cell& c, std::optional<DensePatch>& p) {
        if(p && p->id == id) p = std::nullopt;
    });
}

std::vector<Cell> DenseSlice::get_neigbours_within_slice(const Cell& cell) const
{
    return cell.get_neigbours_within_bounding_box_inclusive({0,0}, get_layout().furthest_cell());
}

DenseSlice::DenseSlice(const Layout& layout)
: cells(std::vector<RowStore>(
        layout.furthest_cell().row+1,
        RowStore(layout.furthest_cell().col+1, std::nullopt))),
  layout(std::cref(layout))
{
}

DenseSlice::DenseSlice(const lsqecc::Layout &layout, const tsl::ordered_set<PatchId> &core_qubit_ids)
 : DenseSlice(layout)
{
    if (layout.core_patches().size()<core_qubit_ids.size())
        throw std::runtime_error("Not enough Init patches for all ids");

    auto core_qubit_ids_itr = core_qubit_ids.begin();
    for (const SparsePatch& p : layout.core_patches())
    {
        if(core_qubit_ids_itr == core_qubit_ids.end()) break;
        Cell cell = place_single_cell_sparse_patch(p,false);
        patch_at(cell)->id = *core_qubit_ids_itr++;
    }

    for (const Cell& cell: layout.predistilled_y_states())
    {
        SparsePatch p = LayoutHelpers::basic_square_patch(cell);
        place_single_cell_sparse_patch(p,false);
        predistilled_ystates_available++;
    }

    for(const MultipleCellsOccupiedByPatch& distillation_region: layout.distillation_regions())
    {
        for (const SingleCellOccupiedByPatch& cell: distillation_region.sub_cells)
        {
            patch_at(cell.cell) = DensePatch{
                    Patch{PatchType::Distillation,PatchActivity::Distillation,std::nullopt},
                    static_cast<CellBoundaries>(cell)};
        }
    }

    for (const Cell& cell: layout.reserved_for_magic_states()) 
    {
        patch_at(cell) = DensePatch{
                Patch{PatchType::Distillation,PatchActivity::Distillation,std::nullopt},
                CellBoundaries{Boundary{BoundaryType::Connected, false},Boundary{BoundaryType::Connected, false},
                    Boundary{BoundaryType::Connected, false},Boundary{BoundaryType::Connected, false}}};
    }

    // Reserved tiles are themselves 'distillation regions'
    for(auto t : layout.distillation_times())
        time_to_next_magic_state_by_distillation_region.push_back(t);


    for(const Cell& cell: layout.dead_location())
    {
        patch_at(cell) = DensePatch{
            Patch{PatchType::Dead,PatchActivity::Dead,std::nullopt},
            CellBoundaries{Boundary{BoundaryType::Connected, false},Boundary{BoundaryType::Connected, false},
                Boundary{BoundaryType::Connected, false},Boundary{BoundaryType::Connected, false}}};
    }

}

bool DenseSlice::is_cell_free(const Cell& cell) const
{
    return !patch_at(cell).has_value();
}

Cell DenseSlice::place_single_cell_sparse_patch(const SparsePatch& sparse_patch, bool distillation)
{
    auto* occupied_cell = std::get_if<SingleCellOccupiedByPatch>(&sparse_patch.cells);
    if(!occupied_cell)
        throw std::logic_error("Placing Multi Patch cell not yet supported");
    else {
        if(!is_cell_free(occupied_cell->cell) && !distillation)
            throw std::logic_error(lstk::cat("Double patch occupation at ",occupied_cell->cell, "\n",
                    "Found patch: ", patch_at(occupied_cell->cell)->id.value_or(-1)));
    }

    patch_at(occupied_cell->cell) = DensePatch::from_sparse_patch(sparse_patch);
    return occupied_cell->cell;
}
void DenseSlice::place_sparse_patch(const SparsePatch& sparse_patch, bool distillation)
{
    auto* occupied_cells = std::get_if<MultipleCellsOccupiedByPatch>(&sparse_patch.cells);
    if (!occupied_cells) 
        place_single_cell_sparse_patch(sparse_patch, distillation);
    else 
    {
        for (const SingleCellOccupiedByPatch& patch : occupied_cells->sub_cells)
            place_single_cell_sparse_patch(SparsePatch{.cells=patch}, distillation);
    }
}

bool DenseSlice::has_patch(PatchId id) const
{
    return (bool) get_patch_by_id(id);
}

std::optional<std::reference_wrapper<Boundary>> DenseSlice::get_boundary_between(
        const Cell& target, const Cell& neighbour)
{
    auto& target_patch = patch_at(target);
    if(!target_patch) return std::nullopt;

    if(neighbour == Cell{target.row-1, target.col})   return target_patch->boundaries.top;
    if(neighbour == Cell{target.row+1, target.col})   return target_patch->boundaries.bottom;
    if(neighbour == Cell{target.row,   target.col-1}) return target_patch->boundaries.left;
    if(neighbour == Cell{target.row,   target.col+1}) return target_patch->boundaries.right;

    return std::nullopt;

}

std::reference_wrapper<Boundary> DenseSlice::get_boundary_between_or_fail(const Cell& target, const Cell& neighbour)
{
    return lstk::get_or_throw(get_boundary_between(target, neighbour), 
        std::logic_error{lstk::cat("No boundary between cells ", target, "and ", neighbour)});
}


std::optional<std::reference_wrapper<const Boundary>> DenseSlice::get_boundary_between(
        const Cell& target, const Cell& neighbour) const
{
    std::optional<std::reference_wrapper<Boundary>> r
        = const_cast<DenseSlice*>(this)->get_boundary_between(target,neighbour);
    return r ? std::make_optional(std::cref(r->get())) : std::nullopt;
}

bool DenseSlice::have_boundary_of_type_with(const Cell& target, const Cell& neighbour, PauliOperator op) const
{
    const auto b = get_boundary_between(target, neighbour);
    return b ? b->get().boundary_type== boundary_for_operator(op) : false;
}


SurfaceCodeTimestep DenseSlice::time_to_next_magic_state(size_t distillation_region_id) const
{
    return time_to_next_magic_state_by_distillation_region[distillation_region_id];
}

}

