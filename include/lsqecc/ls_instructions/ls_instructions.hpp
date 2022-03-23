#ifndef LSQECC_LOGICAL_LATTICE_OPS_HPP
#define LSQECC_LOGICAL_LATTICE_OPS_HPP

#include <variant>
#include <stdexcept>
#include <unordered_set>
#include <lsqecc/patches/patches.hpp>

#include <tsl/ordered_map.h>
#include <tsl/ordered_set.h>


namespace lsqecc {


struct DeclareLogicalQubitPatches{
    tsl::ordered_set<PatchId> patch_ids;

    bool operator==(const DeclareLogicalQubitPatches&) const = default;
};

struct SinglePatchMeasurement {
    PatchId target;
    PauliOperator observable;
    bool is_negative;

    bool operator==(const SinglePatchMeasurement&) const = default;
};

struct MultiPatchMeasurement {
    tsl::ordered_map<PatchId, PauliOperator> observable;
    bool is_negative;

    bool operator==(const MultiPatchMeasurement&) const = default;
};

struct PatchInit {
    PatchId target;

    enum class InitializeableStates : uint8_t {
        Zero,
        Plus
    };

    InitializeableStates state;

    bool operator==(const PatchInit&) const = default;
};

struct MagicStateRequest {
    PatchId target;

    bool operator==(const MagicStateRequest&) const = default;
};

struct SingleQubitOp {
    PatchId target;

    enum class Operator : uint8_t {
        X = static_cast<uint8_t>(PauliOperator::X),
        Z = static_cast<uint8_t>(PauliOperator::Z),
        H,
        S,
    };

    Operator op;

    bool operator==(const SingleQubitOp&) const = default;
};

struct LSInstruction {
    std::variant<
            DeclareLogicalQubitPatches,
            SinglePatchMeasurement,
            MultiPatchMeasurement,
            PatchInit,
            MagicStateRequest,
            SingleQubitOp> operation;

    std::vector<PatchId> get_operating_patches() const;
    bool operator==(const LSInstruction&) const = default;
};

struct InMemoryLogicalLatticeComputation
{
    tsl::ordered_set<PatchId> core_qubits;
    std::vector<LSInstruction> instructions;
};


}


#endif //LSQECC_LOGICAL_LATTICE_OPS_HPP
