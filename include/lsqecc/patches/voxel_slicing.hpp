#pragma once

#include <lsqecc/patches/dense_patch_computation.hpp>

namespace lsqecc {


struct VoxelizedInstructionResult
{
    std::vector<DenseSlice> volume_after_application;
    size_t new_slice_count = 0;
};


VoxelizedInstructionResult voxelize_instruction_on_volume(
        std::vector<DenseSlice> volume, // Expensive copy
        LSInstruction instruction,
        bool local_instructions,
        const Layout& layout,
        DenseSliceVisitor slice_visitor);
}