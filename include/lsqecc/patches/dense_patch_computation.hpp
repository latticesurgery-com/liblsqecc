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
#include <functional>
#include <stdexcept>
#include <string_view>

namespace lsqecc {

enum class PipelineMode {
    Stream, Dag, Wave, EDPC
};

using DenseSliceVisitor = std::function<void(const DenseSlice& slice)>;
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
static constexpr size_t MAX_NO_PROGRESS_SLICES_STREAM_DEFAULT = 1000;


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


DensePatchComputationResult run_through_dense_slices(
        LSInstructionStream&& instruction_stream,
        PipelineMode pipeline_mode,
        bool local_instructions,
        bool allow_twists,
        const Layout& layout,
        std::unique_ptr<Router> router,
        std::optional<std::chrono::seconds> timeout,
        DenseSliceVisitor slice_visitor,
        LSInstructionVisitor instruction_visitor,
        bool graceful,
        bool gen_op_ids,
        size_t max_no_progress_slices = MAX_NO_PROGRESS_SLICES_STREAM_DEFAULT);



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