#include <lsqecc/ls_instructions/ls_instructions_from_gates.hpp>

namespace lsqecc
{


LSIinstructionFromGatesGenerator::LSIinstructionFromGatesGenerator(PatchId ancilla_state_id_start)
: ancilla_state_id_counter_(ancilla_state_id_start)
{}

PatchId LSIinstructionFromGatesGenerator::get_next_ancilla_state_id()
{
    return ancilla_state_id_counter_++;
}

std::queue<LSInstruction> LSIinstructionFromGatesGenerator::make_t_gate_instructions(PatchId target_id)
{
    std::queue<LSInstruction> next_instructions;
    PatchId new_magic_state_id = get_next_ancilla_state_id();
    next_instructions.push({.operation={MagicStateRequest{new_magic_state_id, MagicStateRequest::DEFAULT_WAIT}}});
    next_instructions.push({.operation={
            MultiPatchMeasurement{.observable={
                    {target_id,PauliOperator::Z},
                    {new_magic_state_id,PauliOperator::Z},
            },.is_negative=false}}});
    next_instructions.push({.operation={SinglePatchMeasurement{
            new_magic_state_id, PauliOperator::X, false
    }}});
    next_instructions.push({.operation={SingleQubitOp{target_id, SingleQubitOp::Operator::S}}});
    return next_instructions;
}

std::queue<LSInstruction> LSIinstructionFromGatesGenerator::make_cnot_instructions(
        PatchId control_id, PatchId target_id, gates::CNOTType cnot_type, gates::CNOTAncillaPlacement cnot_ancilla_placement)
{
    // Control is green -> smooth -> measures X otimes X
    // Target is red -> rough -> measures Z otimes Z

    std::optional<PatchInit::PlaceNexTo> place_ancilla_next_to;
    if(cnot_ancilla_placement == gates::CNOTAncillaPlacement::ANCILLA_NEXT_TO_CONTROL)
        place_ancilla_next_to = std::make_pair(control_id, PauliOperator::X);
    else if(cnot_ancilla_placement == gates::CNOTAncillaPlacement::ANCILLA_NEXT_TO_TARGET)
        place_ancilla_next_to = std::make_pair(target_id, PauliOperator::Z);

    std::queue<LSInstruction> next_instructions;
    PatchId ancilla_id = get_next_ancilla_state_id();
    next_instructions.push({.operation={PatchInit{
            ancilla_id, PatchInit::InitializeableStates::Plus, place_ancilla_next_to}}});

    if(cnot_type == gates::CNOTType::ZX_WITH_MBM_CONTROL_FIRST){
        next_instructions.push({.operation={
                MultiPatchMeasurement{.observable={
                        {control_id, PauliOperator::X},
                        {ancilla_id, PauliOperator::X},
                },.is_negative=false}}});
        next_instructions.push({.operation={
                MultiPatchMeasurement{.observable={
                        {ancilla_id,PauliOperator::Z},
                        {target_id,PauliOperator::Z},
                },.is_negative=false}}});
    }
    else if(cnot_type == gates::CNOTType::ZX_WITH_MBM_TARGET_FIRST){
        next_instructions.push({.operation={
                MultiPatchMeasurement{.observable={
                        {ancilla_id,PauliOperator::Z},
                        {target_id,PauliOperator::Z},
                },.is_negative=false}}});
        next_instructions.push({.operation={
                MultiPatchMeasurement{.observable={
                        {control_id, PauliOperator::X},
                        {ancilla_id, PauliOperator::X},
                },.is_negative=false}}});
    }
    else LSTK_UNREACHABLE;

    next_instructions.push({.operation={SinglePatchMeasurement{ancilla_id, PauliOperator::X, false}}});

    return next_instructions;
}

}