#ifndef LSQECC_PATCH_COMPUTATION_HPP
#define LSQECC_PATCH_COMPUTATION_HPP


#include <lsqecc/ls_instructions/ls_instructions.hpp>
#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
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
    const Slice& last_slice_const() const {return last_slice_;}
    Slice& second_last_slice() {return second_last_slice_;}
    size_t slice_count() const { return slice_count_;};
private:
    Slice last_slice_;
    Slice second_last_slice_;
    size_t slice_count_ = 0;
};


using FreeCellCache = std::vector<std::vector<lstk::bool8>>;
void compute_free_cells(FreeCellCache& is_cell_free, const Slice& slice);

class PatchComputation
{
public:

    using SliceVisitorFunction = std::function<void(const Slice& slice)>;

    PatchComputation (
            LSInstructionStream&& instruction_stream,
            std::unique_ptr<Layout>&& layout,
            std::unique_ptr<Router>&& router,
            std::optional<std::chrono::seconds> timeout,
            SliceVisitorFunction slice_visitor);

    size_t slice_count() const {return slice_store_.slice_count();}
    size_t ls_instructions_count() const {return ls_instructions_count_;}

private:

    void make_slices(
            LSInstructionStream&& instruction_stream,
            std::optional<std::chrono::seconds> timeout);


    /// Assumes that there already is a slice
    Slice& make_new_slice();

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

#endif //LSQECC_PATCH_COMPUTATION_HPP
