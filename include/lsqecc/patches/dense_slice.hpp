#ifndef LSQECC_DENSE_SLICE_HPP
#define LSQECC_DENSE_SLICE_HPP

#include <lsqecc/patches/patches.hpp>
#include <lsqecc/layout/layout.hpp>
#include <lsqecc/layout/searchable_slice.hpp>

#include <functional>
#include <unordered_map>

namespace lsqecc
{



struct DenseSlice : public SearchableSlice
{
    using SearchableSlice::SearchableSlice;

    using RowStore = std::vector<std::optional<DensePatch>>;
    std::vector<RowStore> cells;

    std::queue<Cell> magic_state_queue;
    std::vector<size_t> time_to_next_magic_state_by_distillation_region;

    explicit DenseSlice(const Layout& layout);

    size_t rows() const {return cells.size();};
    size_t cols() const  {return cells.at(0).size();};
    Cell furthest_cell() const override { return {
        static_cast<Cell::CoordinateType>(rows()-1),
        static_cast<Cell::CoordinateType>(cols()-1)};};

    using CellTraversalFunctor
        = std::function<void(const Cell&, std::optional<DensePatch>&)>;
    using CellTraversalConstFunctor
        = std::function<void(const Cell&, const std::optional<DensePatch>&)>;
    void traverse_cells_mut(const CellTraversalFunctor& f);
    void traverse_cells(const CellTraversalConstFunctor& f) const;

    std::optional<std::reference_wrapper<DensePatch>> get_patch_by_id(PatchId id);
    std::optional<std::reference_wrapper<const DensePatch>> get_patch_by_id(PatchId id) const;
    std::optional<Cell> get_cell_by_id(PatchId id) const override;
    bool has_patch(PatchId id) const override;
    std::optional<DensePatch>& patch_at(const Cell& cell);
    const std::optional<DensePatch>& patch_at(const Cell& cell) const;

    std::optional<std::reference_wrapper<Boundary>> get_boundary_between(const Cell& target, const Cell& neighbour);
    std::optional<std::reference_wrapper<const Boundary>> get_boundary_between(const Cell& target, const Cell& neighbour) const;
    bool have_boundary_of_type_with(const Cell& target, const Cell& neighbour, PauliOperator op) const override;

    // Return a cell of the placed patch
    Cell place_sparse_patch(const SparsePatch& sparse_patch);

    void delete_patch_by_id(PatchId id);

    bool is_cell_free(const Cell& cell) const override;

    std::vector<Cell> get_neigbours_within_slice(const Cell& cell) const override;
};

}

#endif //LSQECC_DENSE_SLICE_HPP
