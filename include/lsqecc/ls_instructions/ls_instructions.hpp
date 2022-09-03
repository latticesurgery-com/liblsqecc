#ifndef LSQECC_LOGICAL_LATTICE_OPS_HPP
#define LSQECC_LOGICAL_LATTICE_OPS_HPP

#include <variant>
#include <stdexcept>
#include <ostream>
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
    size_t wait_at_most_for;

    static const size_t DEFAULT_WAIT = 10;
    bool operator==(const MagicStateRequest&) const = default;
};


struct RotateSingleCellPatch {
    PatchId target;

    bool operator==(const RotateSingleCellPatch&) const = default;
};

// TODO rename to transversal?
struct SingleQubitOp {
    PatchId target;

    enum class Operator : uint8_t {
        X = static_cast<uint8_t>(PauliOperator::X),
        Z = static_cast<uint8_t>(PauliOperator::Z),
        H,
        S
    };

    Operator op;

    bool operator==(const SingleQubitOp&) const = default;
};


struct BusyRegion{
    RoutingRegion region;
    size_t steps_to_clear;
    SparsePatch state_after_clearing;

    bool operator==(const BusyRegion&) const = default;
};


struct LSInstruction {
    std::variant<
            DeclareLogicalQubitPatches,
            SinglePatchMeasurement,
            MultiPatchMeasurement,
            PatchInit,
            MagicStateRequest,
            SingleQubitOp,
            RotateSingleCellPatch,
            BusyRegion
            > operation;

    std::vector<PatchId> get_operating_patches() const;
    bool operator==(const LSInstruction&) const = default;
};

struct InMemoryLogicalLatticeComputation
{
    tsl::ordered_set<PatchId> core_qubits;
    std::vector<LSInstruction> instructions;
};


std::ostream& operator<<(std::ostream& os, const LSInstruction& instruction);

std::ostream& operator<<(std::ostream& os, const DeclareLogicalQubitPatches& instruction);
std::ostream& operator<<(std::ostream& os, const SinglePatchMeasurement& instruction);
std::ostream& operator<<(std::ostream& os, const MultiPatchMeasurement& instruction);
std::ostream& operator<<(std::ostream& os, const PatchInit& instruction);
std::ostream& operator<<(std::ostream& os, const MagicStateRequest& instruction);
std::ostream& operator<<(std::ostream& os, const SingleQubitOp& instruction);
std::ostream& operator<<(std::ostream& os, const RotateSingleCellPatch& instruction);
std::ostream& operator<<(std::ostream& os, const BusyRegion& instruction);

template <class T>
struct LSInstructionPrint{static constexpr std::string_view name = "<LSInstruction>";};


// TODO this mapping is not consistent, go through the codebase and make it so
template<>
struct LSInstructionPrint<DeclareLogicalQubitPatches>{
    static constexpr std::string_view name = "DeclareLogicalQubitPatches";
};

template<>
struct LSInstructionPrint<SinglePatchMeasurement>{
    static constexpr std::string_view name = "MeasureSinglePatch";
};

template<>
struct LSInstructionPrint<MultiPatchMeasurement>{
    static constexpr std::string_view name = "MeasureSinglePatch";
};

template<>
struct LSInstructionPrint<PatchInit>{
    static constexpr std::string_view name = "Init";
};

template<>
struct LSInstructionPrint<MagicStateRequest>{
    static constexpr std::string_view name = "RequestMagicState";
};

template<>
struct LSInstructionPrint<SingleQubitOp>{
    static constexpr std::string_view name = "Gate";
};

template<>
struct LSInstructionPrint<RotateSingleCellPatch>
{
    static constexpr std::string_view name = "RotateSingleCellPatch";
};

template<>
struct LSInstructionPrint<BusyRegion>{
    static constexpr std::string_view name = "BusyRegion";
};



template <PatchInit::InitializeableStates State>
struct InitializeableStatePrint{static constexpr std::string_view name = "<PatchInit::InitializeableState";};

static inline std::string_view InitializeableStates_to_string(PatchInit::InitializeableStates state)
{
    using namespace std::string_view_literals;
    switch(state)
    {
        case PatchInit::InitializeableStates::Zero:
            return "|0>"sv;
        case PatchInit::InitializeableStates::Plus:
            return "|+>"sv;
    }
}


static inline std::string_view SingleQuibitOperatorName_to_string(SingleQubitOp::Operator op)
{
    using namespace std::string_view_literals;
    switch(op)
    {
        case SingleQubitOp::Operator::X: return "X"sv;
        case SingleQubitOp::Operator::Z: return "Z"sv;
        case SingleQubitOp::Operator::H: return "H"sv;
        case SingleQubitOp::Operator::S: return "S"sv;
    }
}


}


#endif //LSQECC_LOGICAL_LATTICE_OPS_HPP
