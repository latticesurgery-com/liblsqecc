#ifndef LSQECC_FAST_PATCH_COMPUTATION_HPP
#define LSQECC_FAST_PATCH_COMPUTATION_HPP


#include <lsqecc/logical_lattice_ops/logical_lattice_ops.hpp>
#include <lsqecc/patches/slice.hpp>
#include <lsqecc/patches/patches.hpp>
#include <lsqecc/layout/layout.hpp>
#include <lsqecc/layout/router.hpp>

#include <lstk/lstk.hpp>

#include <chrono>
#include <functional>

namespace lsqecc {


class SliceStore
{
public:
    explicit SliceStore(const Layout& layout); // start with blank slices no initialization. i.e. slice_count_ = 0
    void accept_new_slice(Slice&& slice);
    Slice& last_slice() {return last_slice_;}
    Slice& second_last_slice() {return second_last_slice_;}
    size_t slice_count() const { return slice_count_;};
private:
    Slice last_slice_;
    Slice second_last_slice_;
    size_t slice_count_ = 0;
};


class PatchComputation
{
public:

    using SliceVisitorFunction = std::function<void(const Slice& slice)>;

    PatchComputation (
            const InMemoryLogicalLatticeComputation& logical_computation,
            std::unique_ptr<Layout>&& layout,
            std::unique_ptr<Router>&& router,
            std::optional<std::chrono::seconds> timeout,
            SliceVisitorFunction slice_visitor);

    size_t slice_count() const {return slice_store_.slice_count();}

private:

    void make_slices(
            const InMemoryLogicalLatticeComputation& logical_computation,
            std::optional<std::chrono::seconds> timeout,
            SliceVisitorFunction slice_visitor);

    /// Assumes that there already is a slice
    Slice& make_new_slice();

    void compute_free_cells();

    std::optional<Cell> find_place_for_magic_state(size_t distillation_region_idx) const;

private: // Data members
    SliceStore slice_store_;

    std::unique_ptr<Layout> layout_;
    std::unique_ptr<Router> router_;

    // Cache which cells are free in the last slice to speed up search computations
    std::vector<std::vector<lstk::bool8>> is_cell_free_;

};


}

#endif //LSQECC_FAST_PATCH_COMPUTATION_HPP
