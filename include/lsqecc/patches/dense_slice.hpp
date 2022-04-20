#ifndef LSQECC_DENSE_SLICE_HPP
#define LSQECC_DENSE_SLICE_HPP

#include <lsqecc/patches/patches.hpp>
#include <lsqecc/layout/layout.hpp>

#include <functional>
#include <unordered_map>

namespace lsqecc
{



struct DenseSlice
{
    using RowStore = std::vector<std::optional<DensePatch>>;
    std::vector<RowStore> cells;

    std::queue<Cell> magic_state_queue;
    std::vector<size_t> time_to_next_magic_state_by_distillation_region;


    size_t rows() const {return cells.size();};
    size_t cols() const  {return cells.at(0).size();};
    Cell furthest_cell() const { return {
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
    std::optional<DensePatch>& patch_at(const Cell& cell);
    const std::optional<DensePatch>& patch_at(const Cell& cell) const;

    // Return a cell of the placed patch
    Cell place_sparse_patch(const SparsePatch& sparse_patch);

    void delete_patch_by_id(PatchId id);

    bool is_cell_free(const Cell& cell) const;

    std::vector<Cell> get_neigbours_within_slice(const Cell& cell) const;

    static DenseSlice make_blank_slice(const Layout& layout);

    bool operator==(const DenseSlice& other) const;

};

}

#endif //LSQECC_DENSE_SLICE_HPP
