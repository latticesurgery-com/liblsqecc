#include <gtest/gtest.h>

#include <lsqecc/patches/dense_slice.hpp>
#include <lsqecc/layout/layout.hpp>

using namespace lsqecc;

namespace {

// Minimal empty layout that just provides a grid of the requested size so we can
// exercise DenseSlice's id->cell cache directly.
struct EmptyTestLayout : public Layout
{
    explicit EmptyTestLayout(Cell furthest) : furthest_(furthest) {}

    Cell furthest_;
    std::vector<SparsePatch> core_patches_;
    std::vector<MultipleCellsOccupiedByPatch> distillation_regions_;
    std::vector<Cell> empty_cells_;
    DistillationTimeMap distillation_times_;

    const std::vector<SparsePatch>& core_patches() const override { return core_patches_; }
    Cell furthest_cell() const override { return furthest_; }
    const std::vector<MultipleCellsOccupiedByPatch>& distillation_regions() const override { return distillation_regions_; }
    const std::vector<Cell>& distilled_state_locations(size_t) const override { return empty_cells_; }
    const DistillationTimeMap& distillation_times() const override { return distillation_times_; }
    const std::vector<Cell>& ancilla_location() const override { return empty_cells_; }
    const std::vector<Cell>& dead_location() const override { return empty_cells_; }
    const std::vector<Cell>& reserved_for_magic_states() const override { return empty_cells_; }
    const std::vector<Cell>& predistilled_y_states() const override { return empty_cells_; }
    const bool magic_states_reserved() const override { return false; }
};

void place_patch(DenseSlice& slice, const Cell& cell, PatchId id)
{
    slice.place_single_cell_sparse_patch(LayoutHelpers::basic_square_patch(cell, id), false);
}

} // namespace


TEST(DenseSliceCache, LookupAfterPlacement)
{
    EmptyTestLayout layout{{3, 3}};
    DenseSlice slice{layout};

    place_patch(slice, Cell{1, 2}, 42);

    EXPECT_EQ(slice.get_cell_by_id(42), (Cell{1, 2}));
    EXPECT_TRUE(slice.has_patch(42));
    EXPECT_FALSE(slice.get_cell_by_id(99).has_value());
}


TEST(DenseSliceCache, AssignPatchIdUpdatesCache)
{
    EmptyTestLayout layout{{3, 3}};
    DenseSlice slice{layout};

    place_patch(slice, Cell{0, 0}, 1);
    slice.assign_patch_id(Cell{0, 0}, 7);

    EXPECT_FALSE(slice.get_cell_by_id(1).has_value());
    EXPECT_EQ(slice.get_cell_by_id(7), (Cell{0, 0}));

    // Clearing the id removes it from the cache
    slice.assign_patch_id(Cell{0, 0}, std::nullopt);
    EXPECT_FALSE(slice.get_cell_by_id(7).has_value());
}


TEST(DenseSliceCache, DeleteAndClearRemoveFromCache)
{
    EmptyTestLayout layout{{3, 3}};
    DenseSlice slice{layout};

    place_patch(slice, Cell{2, 1}, 5);
    slice.delete_patch_by_id(5);
    EXPECT_FALSE(slice.get_cell_by_id(5).has_value());
    EXPECT_FALSE(slice.has_patch(5));

    place_patch(slice, Cell{2, 2}, 6);
    slice.clear_patch_at(Cell{2, 2});
    EXPECT_FALSE(slice.get_cell_by_id(6).has_value());
}


// Regression test: a move reuses the same id at the target cell before the
// source cell is cleared. Clearing the source must not clobber the cache entry
// that now points at the target.
TEST(DenseSliceCache, ReusingIdDuringMoveKeepsCacheConsistent)
{
    EmptyTestLayout layout{{3, 3}};
    DenseSlice slice{layout};

    const Cell source{0, 0};
    const Cell target{0, 1};
    const PatchId id = 3;

    place_patch(slice, source, id);
    ASSERT_EQ(slice.get_cell_by_id(id), source);

    // Place the moved patch (same id) at the target while source still holds the id.
    place_patch(slice, target, id);
    // Now retire the id on the source cell, mirroring the move instruction path.
    slice.assign_patch_id(source, std::nullopt);

    EXPECT_EQ(slice.get_cell_by_id(id), target);
    ASSERT_TRUE(slice.get_patch_by_id(id).has_value());
    EXPECT_EQ(slice.get_patch_by_id(id)->get().get_id(), id);
}


TEST(DenseSliceCache, PlaceDensePatchOverwriteUpdatesCache)
{
    EmptyTestLayout layout{{3, 3}};
    DenseSlice slice{layout};

    place_patch(slice, Cell{1, 1}, 10);
    // Overwrite with a distillation-style placement carrying a new id.
    slice.place_dense_patch_at(Cell{1, 1},
            DensePatch::from_sparse_patch(LayoutHelpers::basic_square_patch(Cell{1, 1}, 11)));

    EXPECT_FALSE(slice.get_cell_by_id(10).has_value());
    EXPECT_EQ(slice.get_cell_by_id(11), (Cell{1, 1}));
}


// Mutating an empty cell means the caller targeted the wrong cell. Failing loudly beats silently
// leaving the lattice unchanged, which previously produced wrong slices much further downstream.
TEST(DenseSliceCache, MutatingAnEmptyCellThrows)
{
    EmptyTestLayout layout{{3, 3}};
    DenseSlice slice{layout};

    const Cell empty{2, 2};
    ASSERT_TRUE(slice.is_cell_free(empty));

    EXPECT_THROW(slice.set_patch_activity(empty, PatchActivity::Measurement), std::logic_error);
    EXPECT_THROW(slice.set_patch_type(empty, PatchType::Qubit), std::logic_error);
    EXPECT_THROW(slice.assign_patch_id(empty, 1), std::logic_error);
    EXPECT_THROW(slice.rotate_patch_boundaries(empty), std::logic_error);

    // Clearing an already-empty cell stays a no-op: callers use it to guarantee emptiness.
    EXPECT_NO_THROW(slice.clear_patch_at(empty));
}


TEST(DenseSliceCache, RotatePatchBoundariesKeepsIdAndCache)
{
    EmptyTestLayout layout{{3, 3}};
    DenseSlice slice{layout};

    const Cell cell{0, 2};
    place_patch(slice, cell, 4);
    const CellBoundaries before = slice.patch_at(cell)->boundaries;

    slice.rotate_patch_boundaries(cell);

    // The patch keeps its identity (and cache entry); only the boundaries move round.
    EXPECT_EQ(slice.get_cell_by_id(4), cell);
    const CellBoundaries after = slice.patch_at(cell)->boundaries;
    EXPECT_EQ(after.top.boundary_type, before.left.boundary_type);
    EXPECT_EQ(after.right.boundary_type, before.top.boundary_type);
}


// The traversal, not the visitor, owns deallocation: the visitor keeps a valid DensePatch& for the
// whole call and the id->cell entry is dropped along with the patch.
TEST(DenseSliceCache, TraversalClearsRequestedCellsAndEvictsCache)
{
    EmptyTestLayout layout{{3, 3}};
    DenseSlice slice{layout};

    place_patch(slice, Cell{0, 0}, 20);
    place_patch(slice, Cell{1, 1}, 21);

    slice.traverse_cells_mut([](const Cell&, DensePatch& p) {
        return p.get_id() == 20 ? DenseSlice::CellVisit::Clear : DenseSlice::CellVisit::Keep;
    });

    EXPECT_TRUE(slice.is_cell_free(Cell{0, 0}));
    EXPECT_FALSE(slice.get_cell_by_id(20).has_value());
    EXPECT_EQ(slice.get_cell_by_id(21), (Cell{1, 1}));
}
