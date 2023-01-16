#ifndef LSQECC_DENSE_PATCH_COMPUTATION_HPP
#define LSQECC_DENSE_PATCH_COMPUTATION_HPP


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
        const Layout& layout,
        Router& router,
        std::optional<std::chrono::seconds> timeout,
        const DenseSliceVisitor& slice_visitor,
        bool graceful,
         // TRL 01/16/22: We use the EDPC layout flag to influence certain choices within this function
        bool edpclayout);

}



#endif