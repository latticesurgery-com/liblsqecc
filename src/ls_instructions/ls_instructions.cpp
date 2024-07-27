#include <lsqecc/ls_instructions/ls_instructions.hpp>

#include <sstream>
#include <vector>


namespace lsqecc {


tsl::ordered_set<PatchId> LSInstruction::get_operating_patches() const
{
    tsl::ordered_set<PatchId> ret = clients;
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
            ret.insert(op.loc1.target);
            ret.insert(op.loc2.target);
        },
        [&](const MagicStateRequest& op){
            ret.insert(op.target);
        },
        [&](const YStateRequest& op){
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
        [&](const BellBasedCNOT& op) {
            ret.insert(op.control);
            ret.insert(op.target);
            ret.insert(op.side1);
            ret.insert(op.side2);
        },
        [&](const DeclareLogicalQubitPatches& op){
            LSTK_NOOP;
        },
        [&](const PatchReset& op){
            ret.insert(op.target);
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
    os << lstk::join(op_patch_mapping,",");

    if (instruction.local_instruction.has_value())
    {
        os << " [";
        os << instruction.local_instruction.value();
        os << "]";

    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const PlaceNexTo& place_next_to)
{
    os << place_next_to.target << ":" << PauliOperator_to_string(place_next_to.op);

    return os;
}

std::ostream& operator<<(std::ostream& os, const PatchInit& instruction)
{
    os << LSInstructionPrint<PatchInit>::name
        << " " << instruction.target << " " << InitializeableStates_to_string(instruction.state);

    if (instruction.place_next_to)
        os << " " << instruction.place_next_to.value();

    return os;
}

std::ostream& operator<<(std::ostream& os, const BellPairInit& instruction)
{
    os << LSInstructionPrint<BellPairInit>::name
        << " " << instruction.side1 << " " << instruction.side2 << " " << instruction.loc1 << "," << instruction.loc2;

    if (instruction.counter.has_value() && instruction.local_instructions.has_value()) 
    {
        os << " [";
        for (unsigned int i = instruction.counter->first; i < instruction.counter->second; i++) 
        {
            os << instruction.local_instructions.value()[i];
            if (i != instruction.counter->second - 1) 
                os << ";";
        }
        os << "]";

    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const MagicStateRequest& instruction)
{
    return os << LSInstructionPrint<MagicStateRequest>::name
        << " " << instruction.target << " " << instruction.near_patch;
}

std::ostream& operator<<(std::ostream& os, const YStateRequest& instruction)
{
    os << LSInstructionPrint<YStateRequest>::name
        << " " << instruction.target << " " << instruction.near_patch;

    if (instruction.local_instruction.has_value())
    {
        os << " [";
        os << instruction.local_instruction.value();
        os << "]";

    }

    return os;
    
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

std::ostream& operator<<(std::ostream& os, const BellBasedCNOT& instruction)
{
    os << LSInstructionPrint<BellBasedCNOT>::name
        << " " << instruction.control << " " << instruction.target << " " << instruction.side1 << " " << instruction.side2;

    if (instruction.counter_pairs.has_value() && instruction.local_instruction_sets.has_value()) 
    {
        os << " [";
        for (size_t phase : {0, 1})
        {
            for (unsigned int i = instruction.counter_pairs.value()[phase].first; i < instruction.counter_pairs.value()[phase].second; i++) 
            {
                os << instruction.local_instruction_sets.value()[phase][i];
                if (i != instruction.counter_pairs.value()[phase].second - 1) 
                    os << ";";
            }
        }
        os << "]";
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const PatchReset& instruction)
{
    return os << LSInstructionPrint<PatchReset>::name << " " << instruction.target;
}

}