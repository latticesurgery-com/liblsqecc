#ifndef LSQECC_LOGICAL_LATTICE_OPS_HPP
#define LSQECC_LOGICAL_LATTICE_OPS_HPP

#include <variant>
#include <stdexcept>
#include <unordered_set>
#include <lsqecc/patches/patches.hpp>

#include <tsl/ordered_map.h>


namespace lsqecc {



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

struct MagicStateRequest {
    PatchId target;

    bool operator==(const MagicStateRequest&) const = default;
};

struct SingleQubitOp {
    PatchId target;

    enum class Operator {
        X = static_cast<int>(PauliOperator::X),
        Y = static_cast<int >(PauliOperator::Y),
        H,
        S,
    };

    Operator op;

    bool operator==(const SingleQubitOp&) const = default;
};

struct LogicalLatticeOperation {
    std::variant<SinglePatchMeasurement, MultiPatchMeasurement, MagicStateRequest, SingleQubitOp> operation;

    std::vector<PatchId> get_operating_patches() const;
    bool operator==(const LogicalLatticeOperation&) const = default;
};

struct LogicalLatticeComputation
{
    std::unordered_set<PatchId> core_qubits;
    std::vector<LogicalLatticeOperation> instructions;
};


}


#endif //LSQECC_LOGICAL_LATTICE_OPS_HPP
