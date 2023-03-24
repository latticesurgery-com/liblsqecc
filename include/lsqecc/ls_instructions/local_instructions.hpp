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

namespace LocalInstruction
{

struct BellPrepare {
    std::optional<PatchId> side1;
    std::optional<PatchId> side2;
    Cell cell1;
    Cell cell2;

    bool operator==(const BellPrepare&) const = default;
};

struct BellMeasure {
    Cell cell1;
    Cell cell2;

    bool operator==(const BellMeasure&) const = default;
};

struct TwoPatchMeasure {
    Cell cell1;
    Cell cell2;

    bool operator==(const TwoPatchMeasure&) const = default;
};

struct ExtendSplit {
    std::optional<PatchId> side1;
    std::optional<PatchId> side2;
    Cell target_cell;
    Cell extension_cell;

    bool operator==(const ExtendSplit&) const = default;
};

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
