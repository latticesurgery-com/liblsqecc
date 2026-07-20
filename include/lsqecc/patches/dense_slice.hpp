#ifndef LSQECC_DENSE_SLICE_HPP
#define LSQECC_DENSE_SLICE_HPP

#include <lsqecc/patches/patches.hpp>
#include <lsqecc/layout/layout.hpp>
#include <lsqecc/patches/slice.hpp>

#include <functional>
#include <unordered_map>
#include <set>
#include <tsl/ordered_set.h>

namespace lsqecc
{



struct DenseSlice : public Slice
{
    using Slice::Slice;

    using RowStore = std::vector<std::optional<DensePatch>>;
private:
    // Source of truth for patch placement. Kept private so the only way to mutate cell occupancy
    // or patch ids is through the cache-safe helpers below, which keep patch_id_to_cell_cache in
    // sync. Read access is via patch_at() / traverse_cells().
    std::vector<RowStore> cells;
public:
    std::set<Cell> magic_states;
    DistillationTimeMap time_to_next_magic_state_by_distillation_region;
    std::reference_wrapper<const Layout> layout;
    unsigned int predistilled_ystates_available = 0;
    std::set<Cell> EDPC_crossing_vertices;
    // std::vector<std::pair<Cell, Cell>> marked_rough_boundaries_EDPC;
    // std::vector<std::pair<Cell, Cell>> marked_smooth_boundaries_EDPC;
    std::vector<std::reference_wrapper<Boundary>> marked_rough_boundaries_EDPC;
    std::vector<std::reference_wrapper<Boundary>> marked_smooth_boundaries_EDPC;


    explicit DenseSlice(const Layout& layout);
    DenseSlice(const Layout& layout, const tsl::ordered_set<PatchId>& core_qubit_ids);

    virtual const Layout& get_layout() const override;

    // Outcome of visiting an occupied cell during a mutable traversal.
    enum class CellVisit { Keep, Clear };

    // The mutable traversal only exposes *occupied* cells, as a DensePatch&: callers cannot clear or
    // replace a slot (which would bypass the id->cell cache) -- use place_dense_patch_at for
    // replacement, or return CellVisit::Clear to have the traversal drop the patch (and its cache
    // entry) once the visitor has returned, so the visitor never holds a dangling DensePatch&.
    // The const traversal still visits every cell, including empty ones.
    using CellTraversalFunctor
        = std::function<CellVisit(const Cell&, DensePatch&)>;
    using CellTraversalConstFunctor
        = std::function<void(const Cell&, const std::optional<DensePatch>&)>;
    void traverse_cells_mut(const CellTraversalFunctor& f);
    void traverse_cells(const CellTraversalConstFunctor& f) const;

    std::optional<std::reference_wrapper<const DensePatch>> get_patch_by_id(PatchId id) const;
    std::optional<SparsePatch> get_sparse_patch_by_id(PatchId id) const;
    std::optional<Cell> get_cell_by_id(PatchId id) const override;
    bool has_patch(PatchId id) const override;
    const std::optional<DensePatch>& patch_at(const Cell& cell) const;
    // Use cache-safe helpers when replacing/removing patches so id->cell map stays consistent.
    void place_dense_patch_at(const Cell& cell, const DensePatch& patch);
    void clear_patch_at(const Cell& cell);
    // The mutators below require an occupied cell and throw std::logic_error otherwise, so that a
    // mis-targeted write fails loudly rather than silently leaving the lattice unchanged.
    void assign_patch_id(const Cell& cell, std::optional<PatchId> new_id);
    void set_patch_activity(const Cell& cell, PatchActivity activity);
    void set_patch_type(const Cell& cell, PatchType type);
    void rotate_patch_boundaries(const Cell& cell);

    std::optional<std::reference_wrapper<Boundary>> get_boundary_between(const Cell& target, const Cell& neighbour);
    std::reference_wrapper<Boundary> get_boundary_between_or_fail(const Cell& target, const Cell& neighbour);
    std::optional<std::reference_wrapper<const Boundary>> get_boundary_between(const Cell& target, const Cell& neighbour) const;
    bool have_boundary_of_type_with(const Cell& target, const Cell& neighbour, PauliOperator op) const override;
    bool is_boundary_reserved(const Cell& target, const Cell& neighbour) const override;

    // Return a cell of the placed patch
    Cell place_single_cell_sparse_patch(const SparsePatch& sparse_patch, bool distillation);
    void place_sparse_patch(const SparsePatch& sparse_patch, bool distillation);

    void delete_patch_by_id(PatchId id);

    bool is_cell_free(const Cell& cell) const override;
    bool is_cell_free_or_activity(const Cell& cell, std::vector<PatchActivity> activities) const override;

    std::optional<Cell> get_directional_neighbor_within_slice(const Cell& cell, CellDirection dir) const override;
    std::vector<Cell> get_neigbours_within_slice(const Cell& cell) const override;

    SurfaceCodeTimestep time_to_next_magic_state(size_t distillation_region_id) const override;

    void flip_crossing_chain(const Cell& crossing_cell, CellDirection dir);

    BoundaryType mark_boundaries_for_crossing_cell(const SingleCellOccupiedByPatch& p, const Cell& prev);

private:
    // Secondary index over `cells`: maps a patch id to the cell that most recently held it.
    // Invariant: for every entry, the mapped cell holds a patch whose id == key. Maintained by the
    // cache-safe mutators (assign_patch_id / place_dense_patch_at / clear_patch_at). Marked mutable
    // so get_cell_by_id can drop a stale entry it detects on read (self-healing).
    mutable std::unordered_map<PatchId, Cell, std::hash<PatchId>> patch_id_to_cell_cache;
    // Avoid exposing mutable reference to DensePatch as it may lead to breaking the cache
    std::optional<DensePatch>& _patch_at_mut(const Cell& cell);
    // Same, but for mutators that only make sense on an occupied cell. `context` names the caller
    // in the exception message.
    DensePatch& _occupied_patch_at_or_fail(const Cell& cell, const char* context);
    // Only drop the id->cell mapping when it still points at `cell`. Guards against clobbering a
    // mapping that was already re-pointed to another cell (e.g. a move that reuses the same id).
    void _evict_id_cache_entry(PatchId id, const Cell& cell);
};

}

#endif //LSQECC_DENSE_SLICE_HPP
