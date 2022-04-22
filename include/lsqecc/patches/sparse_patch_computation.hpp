#ifndef LSQECC_SPARSE_PATCH_COMPUTATION_HPP
#define LSQECC_SPARSE_PATCH_COMPUTATION_HPP


#include <lsqecc/ls_instructions/ls_instructions.hpp>
#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
#include <lsqecc/patches/patch_computation_result.hpp>
#include <lsqecc/patches/sparse_slice.hpp>
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
    void accept_new_slice(SparseSlice&& slice);
    SparseSlice& last_slice() {return last_slice_;}
    const SparseSlice& last_slice_const() const {return last_slice_;}
    SparseSlice& second_last_slice() {return second_last_slice_;}
    size_t slice_count() const { return slice_count_;};
private:
    SparseSlice last_slice_;
    SparseSlice second_last_slice_;
    size_t slice_count_ = 0;
};


using FreeCellCache = std::vector<std::vector<lstk::bool8>>;
void compute_free_cells(FreeCellCache& is_cell_free, const SparseSlice& slice);

class SparsePatchComputation : public PatchComputationResult
{
public:

    using SliceVisitorFunction = std::function<void(const SparseSlice& slice)>;

    SparsePatchComputation (
            LSInstructionStream&& instruction_stream,
            std::unique_ptr<Layout>&& layout,
            std::unique_ptr<Router>&& router,
            std::optional<std::chrono::seconds> timeout,
            SliceVisitorFunction slice_visitor);

    size_t slice_count() const override {return slice_store_.slice_count();}
    size_t ls_instructions_count() const override {return ls_instructions_count_;}

private:

    void make_slices(
            LSInstructionStream&& instruction_stream,
            std::optional<std::chrono::seconds> timeout);


    /// Assumes that there already is a slice
    SparseSlice& make_new_slice();

    std::optional<Cell> find_place_for_magic_state(size_t distillation_region_idx) const;

private: // Data members
    SliceStore slice_store_;
    SliceVisitorFunction slice_visitor_;

    std::unique_ptr<Layout> layout_;
    std::unique_ptr<Router> router_;

    // Cache which cells are free in the last slice to speed up search computations
    mutable FreeCellCache is_cell_free_;

    size_t ls_instructions_count_ = 0;

};


}

#endif //LSQECC_SPARSE_PATCH_COMPUTATION_HPP
