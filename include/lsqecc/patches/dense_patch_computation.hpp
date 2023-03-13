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
        bool dag_pipeline,
        const Layout& layout,
        Router& router,
        std::optional<std::chrono::seconds> timeout,
        DenseSliceVisitor slice_visitor,
        LSInstructionVisitor instruction_visitor,
        bool graceful);


static constexpr size_t MAX_INSTRUCTION_APPLICATION_RETRIES_DAG_PIPELINE = 10;


}



#endif