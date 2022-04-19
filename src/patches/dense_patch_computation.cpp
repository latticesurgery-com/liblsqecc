#include <lsqecc/patches/dense_patch_computation.hpp>

namespace lsqecc
{


DenseSlice first_slice_from_layout(const Layout& layout, const tsl::ordered_set<PatchId>& core_qubit_ids)
{
    DenseSlice slice = DenseSlice::make_blank_slice(layout);

    for (const SparsePatch& p : layout.core_patches())
    {
        auto* occupied_cell = std::get_if<SingleCellOccupiedByPatch>(&p.cells);
        if(!occupied_cell)
            throw std::logic_error("Patch is not single cell");
        slice.patch_at(occupied_cell->cell) = DensePatch::from_sparse_patch(p);
    }


}


void run_through_dense_slices(
        LSInstructionStream&& instruction_stream,
        const Layout& layout,
        const Router& router,
        std::optional<std::chrono::seconds> timeout,
        const DenseSliceVisitor& slice_visitor)
{

}

}