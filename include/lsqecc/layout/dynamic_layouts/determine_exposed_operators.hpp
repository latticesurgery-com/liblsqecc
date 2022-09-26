#ifndef LSQECC_DETERMINE_EXPOSED_OPERATORS_HPP
#define LSQECC_DETERMINE_EXPOSED_OPERATORS_HPP

#include <lsqecc/pauli_rotations/pauli_operator.hpp>
#include <lsqecc/patches/patches.hpp>
#include <lsqecc/patches/slice.hpp>
#include <lsqecc/layout/layout.hpp>
#include <tsl/ordered_map.h>

namespace lsqecc
{

struct RotatableSingleQubitPatchExposedOperators
{
    bool x_exposed;
    bool z_exposed;

    bool is_exposed(const PauliOperator op) const;
    void rotate();
};

tsl::ordered_map<PatchId, RotatableSingleQubitPatchExposedOperators> determine_exposed_core_operators(const Layout& layout);

RotatableSingleQubitPatchExposedOperators determine_operators_exposed_for_patch(const SparsePatch& patch, const Slice& slice);

}

#endif //LSQECC_DETERMINE_EXPOSED_OPERATORS_HPP
