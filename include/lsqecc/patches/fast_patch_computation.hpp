#ifndef LSQECC_FAST_PATCH_COMPUTATION_HPP
#define LSQECC_FAST_PATCH_COMPUTATION_HPP


#include <lsqecc/logical_lattice_ops/logical_lattice_ops.hpp>
#include <lsqecc/patches/slice.hpp>
#include <lsqecc/patches/patches.hpp>
#include <lsqecc/layout/layout.hpp>
#include <lsqecc/layout/router.hpp>

#include <lstk/lstk.hpp>

#include <chrono>

namespace lsqecc {


enum class SliceTrackingPolicy
{
    KeepAll, KeepOnlyLastTwo
};

class PatchComputation
{
public:

    PatchComputation (
            const LogicalLatticeComputation& logical_computation,
            std::unique_ptr<Layout>&& layout,
            std::unique_ptr<Router>&& router,
            std::optional<std::chrono::seconds> timeout,
            SliceTrackingPolicy slice_tracking_policy);

    const std::vector<Slice>& get_slices() const; // Requires KeepAll policy or throws an error
    size_t slice_count() const;

    const Layout& get_layout() const {return *layout_;}

private:

    void make_slices(const LogicalLatticeComputation& logical_computation, std::optional<std::chrono::seconds> timeout);

    /// Assumes that there already is a slice
    Slice& make_new_slice();
    void accept_new_slice(Slice&& slice);
    Slice& last_slice();
    Slice& second_last_slice();

    void compute_free_cells();
    std::optional<Cell> find_place_for_magic_state(size_t distillation_region_idx) const;

private: // Data members

    std::unique_ptr<Layout> layout_;
    std::unique_ptr<Router> router_;

    using SlicesVector = std::vector<Slice>;
    using SlicesPair = std::pair<Slice,Slice>;
    std::variant<SlicesVector, SlicesPair> slices_;
    size_t slice_count_ = 0;

    // Cache which cells are free in the last slice to speed up search computations
    std::vector<std::vector<lstk::bool8>> is_cell_free_;

};


}

#endif //LSQECC_FAST_PATCH_COMPUTATION_HPP
