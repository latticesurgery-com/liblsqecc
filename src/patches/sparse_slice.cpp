#include <lsqecc/patches/sparse_slice.hpp>
#include <iterator>
#include <algorithm>
#include <iostream>

namespace lsqecc{


const Layout& SparseSlice::get_layout() const
{
    return layout.get();
}

bool SparseSlice::has_patch(PatchId id) const
{
    for(auto& p: qubit_patches)
        if(p.id == id)
            return true;
    return false;
}

SparsePatch& SparseSlice::get_patch_by_id_mut(PatchId id)
{
    for(auto& p: qubit_patches)
        if(p.id == id)
            return p;

    throw std::logic_error(std::string{"No patch for id: "+std::to_string(id)});
}

const SparsePatch& SparseSlice::get_patch_by_id(PatchId id) const
{
    return const_cast<SparseSlice*>(this)->get_patch_by_id_mut(id);
}

void SparseSlice::delete_qubit_patch(PatchId id)
{
    auto new_end = std::remove_if(qubit_patches.begin(), qubit_patches.end(), [&id](const SparsePatch& p){return p.id==id;});
    qubit_patches.erase(new_end,qubit_patches.end());
}


std::optional<std::reference_wrapper<const SparsePatch>> SparseSlice::get_qubit_patch_on_cell(const Cell& cell) const
{
    for(const SparsePatch& p: qubit_patches)
        for (const Cell& c: p.get_cells())
            if(c==cell)
                return p;

    return std::nullopt;
}


std::optional<std::reference_wrapper<const SparsePatch>> SparseSlice::get_magic_state_on_cell(const Cell& cell) const
{
    for(const SparsePatch& p: unbound_magic_states)
        for (const Cell& c: p.get_cells())
            if(c==cell)
                return p;

    return std::nullopt;
}

std::optional<std::reference_wrapper<const SparsePatch>> SparseSlice::get_any_patch_on_cell(const Cell& cell) const
{
    auto p = get_qubit_patch_on_cell(cell);
    if(p) return p;
    return get_magic_state_on_cell(cell);
}

bool SparseSlice::is_cell_free(const Cell& cell) const
{
    for(const auto& distillation_region: layout.get().distillation_regions())
        for(const auto& distillation_cell: distillation_region.sub_cells)
            if(distillation_cell.cell==cell)
                return false;

    return !SparseSlice::get_any_patch_on_cell(cell);
}

bool SparseSlice::is_cell_free_or_EDPC(const Cell& cell) const
{
    throw std::logic_error("PatchActivity::EDPC not implemented for SparseSlice.");
}

std::vector<Cell> SparseSlice::get_neigbours_within_slice(const Cell& cell) const
{
    return cell.get_neigbours_within_bounding_box_inclusive({0,0}, layout.get().furthest_cell());
}
SparseSlice SparseSlice::make_blank_slice(const Layout& layout)
{
    return SparseSlice{
        {},
        {},
        {},
        {layout},
        {},
    };
}

const SingleCellOccupiedByPatch &SparseSlice::get_single_cell_occupied_by_patch_by_id(PatchId id) const
{
    return const_cast<SparseSlice*>(this)->get_single_cell_occupied_by_patch_by_id_mut(id);

}
SingleCellOccupiedByPatch &SparseSlice::get_single_cell_occupied_by_patch_by_id_mut(PatchId id)
{

    SingleCellOccupiedByPatch* patch = std::get_if<SingleCellOccupiedByPatch>(&get_patch_by_id_mut(id).cells);
    if (patch==nullptr) throw std::logic_error("Patch "+std::to_string(id)+" does not hold a single cell");
    return *patch;

}
std::optional<Cell> SparseSlice::get_cell_by_id(PatchId id) const
{
    if (!has_patch(id))
        return std::nullopt;

    return get_patch_by_id(id).get_a_cell();
}


bool SparseSlice::have_boundary_of_type_with(const Cell& target, const Cell& neighbour,
        PauliOperator op) const
{
    const auto* occupied_cell = std::get_if<SingleCellOccupiedByPatch>(&get_qubit_patch_on_cell(target).value().get().cells);
    if(!occupied_cell) return false;
    return occupied_cell->have_boundary_of_type_with(op, neighbour);
}

bool SparseSlice::is_boundary_reserved(const Cell& target, const Cell& neighbor) const 
{
    const auto* occupied_cell = std::get_if<SingleCellOccupiedByPatch>(&get_qubit_patch_on_cell(target).value().get().cells);
    if(!occupied_cell) return false;
    std::optional<Boundary> boundary = occupied_cell->get_boundary_with(neighbor);
    if (boundary && ((boundary->boundary_type==BoundaryType::Reserved_Label1) || (boundary->boundary_type==BoundaryType::Reserved_Label2))) 
        return true;
    else return false;
}

SurfaceCodeTimestep SparseSlice::time_to_next_magic_state(size_t distillation_region_id) const
{
    return time_to_next_magic_state_by_distillation_region[distillation_region_id];
}

}
