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
            if(op.state_after_clearing.id) ret.insert(*op.state_after_clearing.id);
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
        os << "(" << cell.cell.row << "," << cell.cell.col << "),";
    return os << "StepsToClear(" << instruction.steps_to_clear <<")";
}

}