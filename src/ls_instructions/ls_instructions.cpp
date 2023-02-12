#include <lsqecc/ls_instructions/ls_instructions.hpp>

#include <sstream>
#include <vector>


namespace lsqecc {


std::vector<PatchId> LSInstruction::get_operating_patches() const
{
    std::vector<PatchId> ret;
    if (const auto* s = std::get_if<SinglePatchMeasurement>(&operation))
    {
        ret.push_back(s->target);
    }
    else if (const auto* p = std::get_if<SingleQubitOp>(&operation))
    {
        ret.push_back(p->target);
    }
    else if (const auto* m = std::get_if<MultiPatchMeasurement>(&operation))
    {
        for(auto pair : m->observable)
        {
            ret.push_back(pair.first);
        }
    }
    else if (const auto* rotation = std::get_if<RotateSingleCellPatch>(&operation)) {
        ret.push_back(rotation->target);
    }
    else {
        const auto& mr = std::get<MagicStateRequest>(operation);
        ret.push_back(mr.target);
    }
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
        os << "OccupiedRegion:" << "(" << cell.cell.row << "|" << cell.cell.col << ")";
    // TODO: add state_after_clearing if needed. Requires printing a SparsePatch
    return os << "," << "StepsToClear(" << instruction.steps_to_clear <<")";
}

}