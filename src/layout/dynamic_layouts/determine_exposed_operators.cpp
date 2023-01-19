#include <lsqecc/layout/dynamic_layouts/determine_exposed_operators.hpp>
#include <lsqecc/patches/dense_slice.hpp>

namespace lsqecc
{

bool RotatableSingleQubitPatchExposedOperators::is_exposed(const PauliOperator op) const
{
    if(op == PauliOperator::X) return x_exposed;
    if(op == PauliOperator::Z) return z_exposed;
    return false;
}

void RotatableSingleQubitPatchExposedOperators::rotate()
{
    x_exposed = !x_exposed;
    z_exposed = !z_exposed;
}

tsl::ordered_map<PatchId, RotatableSingleQubitPatchExposedOperators> determine_exposed_core_operators(
        const Layout& layout, const tsl::ordered_set<PatchId>& core_qubit_ids)
{
    tsl::ordered_map<PatchId, RotatableSingleQubitPatchExposedOperators> out;

    bool dummy = 0;
    DenseSlice first_slice{layout, core_qubit_ids, dummy};

    for(const PatchId& patch_id : core_qubit_ids)
    {
        out[patch_id] = determine_operators_exposed_for_patch(first_slice.get_sparse_patch_by_id(patch_id).value(), first_slice);
    }
    return out;
}



RotatableSingleQubitPatchExposedOperators determine_operators_exposed_for_patch(const SparsePatch& patch, const Slice& slice)
{
    RotatableSingleQubitPatchExposedOperators out{false, false};

    if(const auto* cell = get_if<SingleCellOccupiedByPatch>(&patch.cells))
    {
        for( const Cell& neighbour : slice.get_neigbours_within_slice(cell->cell))
        {
            if(!slice.is_cell_free(neighbour)) continue;

            if(cell->get_boundary_with(neighbour))
            {
                if(cell->have_boundary_of_type_with(PauliOperator::X, neighbour))
                    out.x_exposed = true;
                if(cell->have_boundary_of_type_with(PauliOperator::Z, neighbour))
                    out.z_exposed = true;
            }
        }
    }
    else LSTK_NOT_IMPLEMENTED;
    return out;
}

}