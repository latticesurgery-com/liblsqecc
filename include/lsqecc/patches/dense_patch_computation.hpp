#ifndef LSQECC_DENSE_PATCH_COMPUTATION_HPP
#define LSQECC_DENSE_PATCH_COMPUTATION_HPP


#include <lsqecc/ls_instructions/ls_instructions.hpp>
#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
#include <lsqecc/patches/dense_slice.hpp>
#include <lsqecc/patches/patches.hpp>
#include <lsqecc/layout/layout.hpp>
#include <lsqecc/layout/router.hpp>

#include <lstk/lstk.hpp>

#include <chrono>
#include <functional>

namespace lsqecc {


using DenseSliceVisitor = std::function<void(const DenseSlice& slice)>;

void run_through_dense_slices(
        LSInstructionStream&& instruction_stream,
        const Layout& layout,
        const Router& router,
        std::optional<std::chrono::seconds> timeout,
        const DenseSliceVisitor& slice_visitor);

}



#endif