#ifndef LSQECC_LOCAL_LATTICE_OPS_HPP
#define LSQECC_LOCAL_LATTICE_OPS_HPP

#include <variant>
#include <stdexcept>
#include <ostream>
#include <unordered_set>

#include <tsl/ordered_map.h>
#include <tsl/ordered_set.h>

#include <lsqecc/patches/patches.hpp>
#include <lsqecc/dag/commutation_trait.hpp>


namespace lsqecc {

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

std::ostream& operator<<(std::ostream& os, const LSInstruction& instruction);
std::ostream& operator<<(std::ostream& os, const BellPrepare& instruction);
std::ostream& operator<<(std::ostream& os, const BellMeasure& instruction);
std::ostream& operator<<(std::ostream& os, const Move& instruction);
std::ostream& operator<<(std::ostream& os, const TwoPatchMeasure& instruction);
std::ostream& operator<<(std::ostream& os, const ExtendSplit& instruction);

template <class T>
struct LocalInstructionPrint{};

template<>
struct LocalInstructionPrint<LocalInstruction::BellPrepare>{
    static constexpr std::string_view name = "BellPrepare";
};

template<>
struct LocalInstructionPrint<LocalInstruction::BellMeasure>{
    static constexpr std::string_view name = "BellMeasure";
};

template<>
struct LocalInstructionPrint<LocalInstruction::TwoPatchMeasure>{
    static constexpr std::string_view name = "TwoPatchMeasure";
};

template<>
struct LocalInstructionPrint<LocalInstruction::ExtendSplit>{
    static constexpr std::string_view name = "ExtendSplit";
};

template<>
struct LocalInstructionPrint<LocalInstruction::Move>{
    static constexpr std::string_view name = "Move";
};
} // namespace LocalInstruction




} // namespace lsqecc


#endif //LSQECC_LOCAL_LATTICE_OPS_HPP
