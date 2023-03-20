#include <lsqecc/ls_instructions/ls_instructions.hpp>

#include <sstream>
#include <vector>


namespace lsqecc {


tsl::ordered_set<PatchId> LSInstruction::get_operating_patches() const
{
    tsl::ordered_set<PatchId> ret;
    std::visit(lstk::overloaded{
        [&](const SinglePatchMeasurement& op){
            ret.insert(op.target);
        },
        [&](const MultiPatchMeasurement& op){
            for(const auto& [patch_id, local_observable] : op.observable)
                ret.insert(patch_id);
        },
        [&](const PatchInit& op){
            ret.insert(op.target);
        },
        [&](const BellPairInit& op) {
            ret.insert(op.side1);
            ret.insert(op.side2);
        },
        [&](const MagicStateRequest& op){
            ret.insert(op.target);
        },
        [&](const SingleQubitOp& op){
            ret.insert(op.target);
        },
        [&](const RotateSingleCellPatch& op){
            ret.insert(op.target);
        },
        [&](const BusyRegion& op){
            for (const SparsePatch& patch : op.state_after_clearing) {
                if(patch.id) ret.insert(*patch.id);
            }
        },
        [&](const DeclareLogicalQubitPatches& op){
            LSTK_NOOP;
        },
        [&](const auto& op){
            LSTK_UNREACHABLE;
        }
    }, operation);
    return ret;
}

std::ostream& operator<<(std::ostream& os, const LSInstruction& instruction)
{
    std::visit([&os](auto&& op){ os << op;}, instruction.operation);
    if (instruction.wait_at_most_for != LSInstruction::DEFAULT_MAX_WAIT)
        os << " #WaitAtMostFor " << instruction.wait_at_most_for;
    return os;
}

std::ostream& operator<<(std::ostream& os, const DeclareLogicalQubitPatches& instruction)
{
    return os << LSInstructionPrint<DeclareLogicalQubitPatches>::name
       << " " << lstk::join(instruction.patch_ids,",");
}

std::ostream& operator<<(std::ostream& os, const SinglePatchMeasurement& instruction)
{
    return os << LSInstructionPrint<SinglePatchMeasurement>::name
     << " " << instruction.target
     << " " << (instruction.is_negative ? "-": "") << PauliOperator_to_string(instruction.observable);
}

std::ostream& operator<<(std::ostream& os, const MultiPatchMeasurement& instruction)
{
    os << LSInstructionPrint<MultiPatchMeasurement>::name << " ";
    if(instruction.is_negative) os << "-";

    std::vector<std::string> op_patch_mapping;
    for(const auto& [patch_id, local_observable] : instruction.observable)
        op_patch_mapping.push_back(lstk::cat(patch_id, ":", PauliOperator_to_string(local_observable)));
    return os << lstk::join(op_patch_mapping,",");
}


std::ostream& operator<<(std::ostream& os, const PatchInit& instruction)
{
    os << LSInstructionPrint<PatchInit>::name
        << " " << instruction.target << " " << InitializeableStates_to_string(instruction.state);

    if (instruction.place_next_to)
        os << " " << instruction.place_next_to->first << ":" << PauliOperator_to_string(instruction.place_next_to->second);

    return os;
}
std::ostream& operator<<(std::ostream& os, const BellPairInit& instruction)
{
    os << LSInstructionPrint<BellPairInit>::name
        << " " << instruction.side1 << " " << instruction.side2 << " " << instruction.loc1.first << ":" << PauliOperator_to_string(instruction.loc1.second)
        << "," << instruction.loc2.first << ":" << PauliOperator_to_string(instruction.loc2.second);

    return os;
}

std::ostream& operator<<(std::ostream& os, const MagicStateRequest& instruction)
{
    return os << LSInstructionPrint<MagicStateRequest>::name
        << " " << instruction.target;
}

std::ostream& operator<<(std::ostream& os, const SingleQubitOp& instruction)
{
    return os << SingleQuibitOperatorName_to_string(instruction.op)
        <<  LSInstructionPrint<SingleQubitOp>::name << " " << instruction.target;
}

std::ostream& operator<<(std::ostream& os, const RotateSingleCellPatch& instruction)
{
    return os << LSInstructionPrint<RotateSingleCellPatch>::name << " " << instruction.target;
}
std::ostream& operator<<(std::ostream& os, const BusyRegion& instruction)
{
    os << LSInstructionPrint<BusyRegion>::name << " ";
    for (const auto &cell: instruction.region.cells)
        os << "OccupiedRegion:" << "(" << cell.cell.row << "," << cell.cell.col << ")";
    return os << " " << "StateAfterClearing:TODO"; // TODO
}

}