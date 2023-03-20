#ifndef LSQECC_DENSE_SLICE_HPP
#define LSQECC_DENSE_SLICE_HPP

#include <lsqecc/patches/patches.hpp>
#include <lsqecc/layout/layout.hpp>
#include <lsqecc/patches/slice.hpp>

#include <functional>
#include <unordered_map>
#include <tsl/ordered_set.h>

namespace lsqecc
{



struct DenseSlice : public Slice
{
    using Slice::Slice;

    using RowStore = std::vector<std::optional<DensePatch>>;
    std::vector<RowStore> cells;
    std::queue<Cell> magic_state_queue;
    DistillationTimeMap time_to_next_magic_state_by_distillation_region;
    std::reference_wrapper<const Layout> layout;

    explicit DenseSlice(const Layout& layout);
    DenseSlice(const Layout& layout, const tsl::ordered_set<PatchId>& core_qubit_ids);

    virtual const Layout& get_layout() const override;

    using CellTraversalFunctor
        = std::function<void(const Cell&, std::optional<DensePatch>&)>;
    using CellTraversalConstFunctor
        = std::function<void(const Cell&, const std::optional<DensePatch>&)>;
    void traverse_cells_mut(const CellTraversalFunctor& f);
    void traverse_cells(const CellTraversalConstFunctor& f) const;

    std::optional<std::reference_wrapper<DensePatch>> get_patch_by_id(PatchId id);
    std::optional<std::reference_wrapper<const DensePatch>> get_patch_by_id(PatchId id) const;
    std::optional<SparsePatch> get_sparse_patch_by_id(PatchId id) const;
    std::optional<Cell> get_cell_by_id(PatchId id) const override;
    bool has_patch(PatchId id) const override;
    std::optional<DensePatch>& patch_at(const Cell& cell);
    const std::optional<DensePatch>& patch_at(const Cell& cell) const;

    std::optional<std::reference_wrapper<Boundary>> get_boundary_between(const Cell& target, const Cell& neighbour);
    std::optional<std::reference_wrapper<const Boundary>> get_boundary_between(const Cell& target, const Cell& neighbour) const;
    bool have_boundary_of_type_with(const Cell& target, const Cell& neighbour, PauliOperator op) const override;

    // Return a cell of the placed patch
    Cell place_sparse_patch(const SparsePatch& sparse_patch, bool distillation);
    void place_sparse_patch_multiple_cells(const SparsePatch& sparse_patch);

    void delete_patch_by_id(PatchId id);

    bool is_cell_free(const Cell& cell) const override;

    std::vector<Cell> get_neigbours_within_slice(const Cell& cell) const override;

    SurfaceCodeTimestep time_to_next_magic_state(size_t distillation_region_id) const override;
};

}

#endif //LSQECC_DENSE_SLICE_HPP
