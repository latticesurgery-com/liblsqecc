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
        bool gen_op_ids);


static constexpr size_t MAX_INSTRUCTION_APPLICATION_RETRIES_DAG_PIPELINE = 100;



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