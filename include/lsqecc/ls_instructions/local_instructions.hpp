#ifndef LSQECC_LOGICAL_LATTICE_OPS_HPP
#define LSQECC_LOGICAL_LATTICE_OPS_HPP

#include <variant>
#include <stdexcept>
#include <ostream>
#include <unordered_set>

#include <tsl/ordered_map.h>
#include <tsl/ordered_set.h>

#include <lsqecc/patches/patches.hpp>
#include <lsqecc/dag/commutation_trait.hpp>


namespace lsqecc {


struct DeclareLogicalQubitPatches{
    tsl::ordered_set<PatchId> patch_ids;

    bool operator==(const DeclareLogicalQubitPatches&) const = default;
};

// TRL 03/24/23: Utilizing a namespace for local instructions
namespace LocalInstruction
{
// TRL 03/22/23: First pass at a new IR for local instructions, which can depend on layout.
// TRL 03/22/23: BellPrepare allocates two sides of a Bell pair at specified adjacent cells
struct BellPrepare {
    std::optional<PatchId> side1;
    std::optional<PatchId> side2;
    Cell cell1;
    Cell cell2;

    bool operator==(const BellPrepare&) const = default;
};
// TRL 03/22/23: BellMeasure performs a ZZ or XX measurement and then measures out in the opposite basis
struct BellMeasure {
    Cell cell1;
    Cell cell2;

    bool operator==(const BellMeasure&) const = default;
};
// TRL 03/22/23: TwoPatchMeasure performs a ZZ or XX measurement
struct TwoPatchMeasure {
    Cell cell1;
    Cell cell2;

    bool operator==(const TwoPatchMeasure&) const = default;
};
// TRL 03/22/23: ExtendSplit merges and splits a patch with an adjacent cell
struct ExtendSplit {
    std::optional<PatchId> side1;
    std::optional<PatchId> side2;
    Cell target_cell;
    Cell extension_cell;

    bool operator==(const ExtendSplit&) const = default;
};
// TRL 03/22/23: Move merges and splits a patch with an adjacent cell and then measures out the cell left behind
struct Move {
    std::optional<PatchId> target;
    Cell cell1;
    Cell cell2;

    bool operator==(const Move&) const = default;
};
// TRL 03/22/23: First pass at a new IR for local instructions, which can depend on layout.
struct LSInstruction {

    // static constexpr size_t DEFAULT_MAX_WAIT = 3; // Allows for rotations to finish

    std::variant<
            BellPrepare,
            BellMeasure,
            TwoPatchMeasure,
            ExtendSplit,
            Move
            > operation;

    // size_t wait_at_most_for = DEFAULT_MAX_WAIT;

    // tsl::ordered_set<PatchId> get_operating_patches() const;
    bool operator==(const LSInstruction&) const = default;
};

// TRL 03/22/23: First pass at a new IR for local instructions
std::ostream& operator<<(std::ostream& os, const LSInstruction& instruction);
std::ostream& operator<<(std::ostream& os, const BellPrepare& instruction);
std::ostream& operator<<(std::ostream& os, const BellMeasure& instruction);
std::ostream& operator<<(std::ostream& os, const Move& instruction);
std::ostream& operator<<(std::ostream& os, const TwoPatchMeasure& instruction);
std::ostream& operator<<(std::ostream& os, const ExtendSplit& instruction);
}

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

struct PlaceNexTo {
    PatchId target;
    PauliOperator op;

    bool operator==(const PlaceNexTo&) const = default;
};

struct PatchInit {
    PatchId target;

    enum class InitializeableStates : uint8_t {
        Zero,
        Plus
    };

    InitializeableStates state;
    std::optional<PlaceNexTo> place_next_to = std::nullopt;

    bool operator==(const PatchInit&) const = default;
};
// TRL 03/16/23: Implementing BellPairInit as a new LLI
struct BellPairInit {
    PatchId side1;
    PatchId side2; 
    PlaceNexTo loc1;
    PlaceNexTo loc2;
    // TRL 03/23/23: Creating LocalInstruction vector
    std::optional<std::vector<LocalInstruction::LSInstruction>> local_instructions;
    std::optional<unsigned int> counter;

    bool operator==(const BellPairInit&) const = default;
};

struct MagicStateRequest {
    PatchId target;

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

// TRL 03/20/23: Updated to take a vector of SparsePatches
struct BusyRegion{
    RoutingRegion region;
    size_t steps_to_clear;
    std::vector<SparsePatch> state_after_clearing;

    bool operator==(const BusyRegion&) const = default;
};

struct LSInstruction {

    static constexpr size_t DEFAULT_MAX_WAIT = 3; // Allows for rotations to finish

    std::variant<
            DeclareLogicalQubitPatches,
            SinglePatchMeasurement,
            MultiPatchMeasurement,
            PatchInit,
            // TRL 03/16/23: Implementing BellPairInit as a new LLI
            BellPairInit,
            MagicStateRequest,
            SingleQubitOp,
            RotateSingleCellPatch,
            BusyRegion
            > operation;

    size_t wait_at_most_for = DEFAULT_MAX_WAIT;

    tsl::ordered_set<PatchId> get_operating_patches() const;
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
// TRL 03/21/23: Adding ability to print PlaceNexTo
std::ostream& operator<<(std::ostream& os, const PlaceNexTo& place_next_to);
std::ostream& operator<<(std::ostream& os, const PatchInit& instruction);
// TRL 03/16/23: Implementing BellPairInit as a new LLI
std::ostream& operator<<(std::ostream& os, const BellPairInit& instruction);
std::ostream& operator<<(std::ostream& os, const MagicStateRequest& instruction);
std::ostream& operator<<(std::ostream& os, const SingleQubitOp& instruction);
std::ostream& operator<<(std::ostream& os, const RotateSingleCellPatch& instruction);
std::ostream& operator<<(std::ostream& os, const BusyRegion& instruction);

template <class T>
struct LSInstructionPrint{};


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
    static constexpr std::string_view name = "MultiBodyMeasure";
};

template<>
struct LSInstructionPrint<PatchInit>{
    static constexpr std::string_view name = "Init";
};
// TRL 03/16/23: Implementing BellPairInit as a new LLI
template<>
struct LSInstructionPrint<BellPairInit>{
    static constexpr std::string_view name = "BellPairInit";
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
// TRL 03/22/23: First pass at a new IR for local instructions
template<>
struct LSInstructionPrint<LocalInstruction::BellPrepare>{
    static constexpr std::string_view name = "BellPrepare";
};

template<>
struct LSInstructionPrint<LocalInstruction::BellMeasure>{
    static constexpr std::string_view name = "BellMeasure";
};

template<>
struct LSInstructionPrint<LocalInstruction::TwoPatchMeasure>{
    static constexpr std::string_view name = "TwoPatchMeasure";
};

template<>
struct LSInstructionPrint<LocalInstruction::ExtendSplit>{
    static constexpr std::string_view name = "ExtendSplit";
};

template<>
struct LSInstructionPrint<LocalInstruction::Move>{
    static constexpr std::string_view name = "Move";
};



template <PatchInit::InitializeableStates State>
struct InitializeableStatePrint{};

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
    LSTK_UNREACHABLE;
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
    LSTK_UNREACHABLE;
}


namespace dag {

template<>
struct CommutationTrait<LSInstruction>
{
    static bool can_commute(const LSInstruction& a, const LSInstruction& b)
    {
        return lstk::set_intersection(a.get_operating_patches(), b.get_operating_patches()).empty();
    }
};


} // namespace dag



} // namespace lsqecc


#endif //LSQECC_LOGICAL_LATTICE_OPS_HPP
