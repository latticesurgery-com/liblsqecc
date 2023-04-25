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
#include <lsqecc/ls_instructions/local_instructions.hpp>


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

struct BellPairInit {
    PatchId side1;
    PatchId side2; 
    PlaceNexTo loc1;
    PlaceNexTo loc2;

    std::optional<std::vector<LocalInstruction::LocalLSInstruction>> local_instructions;
    std::optional<std::pair<unsigned int, unsigned int>> counter;

    bool operator==(const BellPairInit&) const = default;
};

struct MagicStateRequest {
    PatchId target;

    static const size_t DEFAULT_WAIT = 10;
    bool operator==(const MagicStateRequest&) const = default;
};

struct YStateRequest {
    PatchId target;
    PatchId near_patch;

    static const size_t DEFAULT_WAIT = 8;
    bool operator==(const YStateRequest&) const = default;
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
    std::vector<SparsePatch> state_after_clearing;

    bool operator==(const BusyRegion&) const = default;
};

struct BellBasedCNOT {
    PatchId control;
    PatchId target;

    std::optional<std::vector<LocalInstruction::LocalLSInstruction>> local_instructions;
    std::optional<std::pair<unsigned int, unsigned int>> counter;

    bool operator==(const BellBasedCNOT&) const = default;
};

struct LSInstruction {

    static constexpr size_t DEFAULT_MAX_WAIT = 3; // Allows for rotations to finish

    std::variant<
            DeclareLogicalQubitPatches,
            SinglePatchMeasurement,
            MultiPatchMeasurement,
            PatchInit,
            BellPairInit,
            MagicStateRequest,
            YStateRequest,
            SingleQubitOp,
            RotateSingleCellPatch,
            BusyRegion,
            BellBasedCNOT
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
std::ostream& operator<<(std::ostream& os, const PlaceNexTo& place_next_to);
std::ostream& operator<<(std::ostream& os, const PatchInit& instruction);
std::ostream& operator<<(std::ostream& os, const BellPairInit& instruction);
std::ostream& operator<<(std::ostream& os, const MagicStateRequest& instruction);
std::ostream& operator<<(std::ostream& os, const YStateRequest& instruction);
std::ostream& operator<<(std::ostream& os, const SingleQubitOp& instruction);
std::ostream& operator<<(std::ostream& os, const RotateSingleCellPatch& instruction);
std::ostream& operator<<(std::ostream& os, const BusyRegion& instruction);
std::ostream& operator<<(std::ostream& os, const BellBasedCNOT& instruction);

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

template<>
struct LSInstructionPrint<BellPairInit>{
    static constexpr std::string_view name = "BellPairInit";
};

template<>
struct LSInstructionPrint<MagicStateRequest>{
    static constexpr std::string_view name = "RequestMagicState";
};

template<>
struct LSInstructionPrint<YStateRequest>{
    static constexpr std::string_view name = "RequestYState";
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

template<>
struct LSInstructionPrint<BellBasedCNOT>{
    static constexpr std::string_view name = "BellBasedCNOT";
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
