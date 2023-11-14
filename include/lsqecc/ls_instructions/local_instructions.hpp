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

struct BellPrepare { // TODO rename to BellPrepareNeighbours
    std::optional<PatchId> side1;
    std::optional<PatchId> side2;
    Cell cell1;
    Cell cell2;

    bool operator==(const BellPrepare&) const = default;
};

struct BellMeasure { // TODO find the difference with the one below and give a more descriptive name
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
    std::optional<PatchId> extension_id;
    Cell target_cell;
    Cell extension_cell;

    bool operator==(const ExtendSplit&) const = default;
};

struct MergeContract {
    Cell preserved_cell;
    Cell measured_cell;

    bool operator==(const MergeContract&) const = default;
};

struct Move {
    Cell source_cell;
    Cell target_cell;
    std::optional<PatchId> new_id_for_target;

    bool operator==(const Move&) const = default;
};

struct LocalLSInstruction {

    std::variant<
            BellPrepare,
            BellMeasure,
            TwoPatchMeasure,
            ExtendSplit,
            MergeContract,
            Move
            > operation;

    bool operator==(const LocalLSInstruction&) const = default;
};

std::ostream& operator<<(std::ostream& os, const LocalLSInstruction& instruction);
std::ostream& operator<<(std::ostream& os, const BellPrepare& instruction);
std::ostream& operator<<(std::ostream& os, const BellMeasure& instruction);
std::ostream& operator<<(std::ostream& os, const Move& instruction);
std::ostream& operator<<(std::ostream& os, const TwoPatchMeasure& instruction);
std::ostream& operator<<(std::ostream& os, const ExtendSplit& instruction);
std::ostream& operator<<(std::ostream& os, const MergeContract& instruction);

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
struct LocalInstructionPrint<LocalInstruction::MergeContract>{
    static constexpr std::string_view name = "MergeContract";
};

template<>
struct LocalInstructionPrint<LocalInstruction::Move>{
    static constexpr std::string_view name = "Move";
};
} // namespace LocalInstruction




} // namespace lsqecc


#endif //LSQECC_LOCAL_LATTICE_OPS_HPP
