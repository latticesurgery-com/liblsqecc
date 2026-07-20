#ifndef LSQECC_DENSE_PATCH_COMPUTATION_HPP
#define LSQECC_DENSE_PATCH_COMPUTATION_HPP


#include <lsqecc/dag/dependency_dag.hpp>
#include <lsqecc/ls_instructions/ls_instructions.hpp>
#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
#include <lsqecc/patches/patch_computation_result.hpp>
#include <lsqecc/patches/dense_slice.hpp>
#include <lsqecc/patches/patches.hpp>
#include <lsqecc/layout/layout.hpp>
#include <lsqecc/layout/router.hpp>

#include <lstk/lstk.hpp>

#include <chrono>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>

namespace lsqecc {

enum class PipelineMode {
    Stream, Dag, Wave, EDPC
};

using LSInstructionVisitor = std::function<void(const LSInstruction& slice)>;

struct DensePatchComputationResult : public PatchComputationResult {
    using PatchComputationResult::PatchComputationResult;

    DensePatchComputationResult(const DensePatchComputationResult& other);

    size_t ls_instructions_count_ = 0;
    size_t slice_count_ = 1;

    size_t ls_instructions_count() const override {return ls_instructions_count_;}
    size_t slice_count() const override {return slice_count_;}
};


static constexpr size_t MAX_INSTRUCTION_APPLICATION_RETRIES_DAG_PIPELINE = 100;

// Max consecutive slices without applying any instruction before a pipeline gives up and
// reports a deadlock (e.g. an operation the layout cannot route).
static constexpr size_t MAX_NO_PROGRESS_SLICES_DEFAULT = 1000;


// Tunables for run_through_dense_slices, grouped so call sites name each knob (designated
// initializers) instead of passing a row of unlabeled positional bools.
struct DenseSlicingOptions {
    PipelineMode pipeline_mode = PipelineMode::Stream;
    bool local_instructions = false;
    bool allow_twists = false;
    bool gen_op_ids = false;
    std::optional<std::chrono::seconds> timeout = std::nullopt;
    size_t max_no_progress_slices = MAX_NO_PROGRESS_SLICES_DEFAULT;
};


// Detects when a pipeline stops making forward progress and aborts with an actionable
// diagnostic instead of looping forever. Shared by the stream and wave pipelines: a stuck
// instruction is retried each slice/wave, so consecutive no-progress slices equal its
// failed retries.
class NoProgressGuard {
public:
    explicit NoProgressGuard(size_t max_no_progress_slices)
        : max_no_progress_slices_(max_no_progress_slices) {}

    // Call when an instruction was applied this slice/wave.
    void note_progress() { slices_without_progress_ = 0; }

    // Call when a slice/wave advanced without applying any instruction. Throws a deadlock
    // diagnostic once the budget is exhausted. When known, `stuck`/`cause` name a
    // representative blocked instruction.
    void note_no_progress(const LSInstruction* stuck = nullptr, std::string_view cause = {})
    {
        if (++slices_without_progress_ <= max_no_progress_slices_)
            return;
        throw std::runtime_error{lstk::cat(
            "No routing progress for ", max_no_progress_slices_,
            " slices; the slicer appears deadlocked.",
            stuck ? lstk::cat("\nCould not apply instruction:\n", *stuck,
                              "\nCaused by:\n", cause)
                  : std::string{},
            "\nThe current layout may be unable to route this operation. "
            "Try a different layout (e.g. -L edpc), or raise the limit with --maxwait.")};
    }

private:
    size_t max_no_progress_slices_;
    size_t slices_without_progress_ = 0;
};


// Pull-based source of dense slices. The slicing engine is inherently sequential (each slice
// is the previous one advanced in place), so this exposes one live DenseSlice that the caller
// drives forward with next(). The classic input-iterator adaptor lets callers use range-for:
//
//     for (const DenseSlice& slice : *stream) { ... }
//
// The reference returned by next()/operator* is only valid until the following next() call,
// which mutates the slice in place (advance_slice). Never copy or retain the DenseSlice.
class DenseSliceStream {
public:
    virtual ~DenseSliceStream() = default;

    // Returns the next slice, or nullptr when exhausted. May throw (deadlock, routing failure,
    // timeout); the exception surfaces here, in the consumer.
    virtual const DenseSlice* next() = 0;

    // Instruction/slice counts, final once the stream is exhausted. Concrete streams accumulate
    // into result_; decorators forward to the stream they wrap.
    virtual const DensePatchComputationResult& result() const { return result_; }

    class iterator {
        DenseSliceStream* stream_ = nullptr;
        const DenseSlice* current_ = nullptr;
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = DenseSlice;
        using reference = const DenseSlice&;
        using pointer = const DenseSlice*;
        using difference_type = std::ptrdiff_t;

        iterator() = default;
        explicit iterator(DenseSliceStream* stream)
            : stream_(stream), current_(stream ? stream->next() : nullptr) {}

        const DenseSlice& operator*() const { return *current_; }
        const DenseSlice* operator->() const { return current_; }
        iterator& operator++() { current_ = stream_->next(); return *this; }

        bool operator!=(const iterator& other) const { return current_ != other.current_; }
        bool operator==(const iterator& other) const { return current_ == other.current_; }
    };

    iterator begin() { return iterator{this}; }
    iterator end() { return iterator{}; }

protected:
    DensePatchComputationResult result_;
};


// Drives the slicing engine and returns a pull-based stream of dense slices. Read the final
// counts from the returned stream's result() once it is exhausted. Exceptions (deadlock, routing
// failure, timeout) propagate out of DenseSliceStream::next(), i.e. into the consumer loop, so
// graceful error handling belongs at the call site.
std::unique_ptr<DenseSliceStream> run_through_dense_slices(
        std::unique_ptr<LSInstructionStream> instruction_stream,
        const Layout& layout,
        std::unique_ptr<Router> router,
        LSInstructionVisitor instruction_visitor,
        const DenseSlicingOptions& options = {});



// TODO replace with a variant
struct InstructionApplicationResult
{
    std::unique_ptr<std::exception> maybe_error;
    std::vector<LSInstruction> followup_instructions;
};

InstructionApplicationResult try_apply_instruction_direct_followup(
        DenseSlice& slice,
        LSInstruction& instruction,
        bool local_instructions,
        bool allow_twists,
        bool gen_op_ids,
        const Layout& layout,
        Router& router);


}



#endif