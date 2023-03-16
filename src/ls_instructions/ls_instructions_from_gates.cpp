#include <lsqecc/ls_instructions/ls_instructions_from_gates.hpp>

namespace lsqecc
{


LSIinstructionFromGatesGenerator::LSIinstructionFromGatesGenerator(IdGenerator& id_generator)
: id_generator_(id_generator)
{}

std::queue<LSInstruction> LSIinstructionFromGatesGenerator::make_t_gate_instructions(PatchId target_id)
{
    std::queue<LSInstruction> next_instructions;
    PatchId new_magic_state_id = id_generator_.new_id();
    next_instructions.push({.operation={MagicStateRequest{new_magic_state_id}}, .wait_at_most_for=MagicStateRequest::DEFAULT_WAIT});
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
        PatchId control_id,
        PatchId target_id,
        gates::CNOTType cnot_type,
        gates::CNOTAncillaPlacement cnot_ancilla_placement,
        CNOTCorrectionMode cnot_correction_mode)
{
    // Control is green -> smooth -> measures Z otimes Z
    // Target is red -> rough -> measures X otimes X

    std::queue<LSInstruction> next_instructions;

    // TRL 03/15/23: Pushing instructions for BELL_BASED CNOT type (control-first with odd number of intervening tiles)
    // TRL 03/16/23: Implementing using BellPairInit as a new LLI
    if (cnot_type == gates::CNOTType::BELL_BASED) {

        // next_instructions.push({.operation={
        //         BellPairInit{id1, id2, control, target}}})

    }
    
    else {

    std::optional<PatchInit::PlaceNexTo> place_ancilla_next_to;
    if(cnot_ancilla_placement == gates::CNOTAncillaPlacement::ANCILLA_NEXT_TO_CONTROL)
        place_ancilla_next_to = std::make_pair(control_id, PauliOperator::Z);
    else if(cnot_ancilla_placement == gates::CNOTAncillaPlacement::ANCILLA_NEXT_TO_TARGET)
        place_ancilla_next_to = std::make_pair(target_id, PauliOperator::X);

        PatchId ancilla_id = id_generator_.new_id();
        next_instructions.push({.operation={PatchInit{
                ancilla_id, PatchInit::InitializeableStates::Plus, place_ancilla_next_to}}});

        if(cnot_type == gates::CNOTType::ZX_WITH_MBM_CONTROL_FIRST){
                next_instructions.push({.operation={
                        MultiPatchMeasurement{.observable={
                                {control_id, PauliOperator::Z},
                                {ancilla_id, PauliOperator::Z},
                        },.is_negative=false}}});
                if(cnot_correction_mode == CNOTCorrectionMode::ALWAYS)
                next_instructions.push({.operation={
                        SingleQubitOp{ancilla_id, SingleQubitOp::Operator::X}
                }});
                next_instructions.push({.operation={
                        MultiPatchMeasurement{.observable={
                                {ancilla_id,PauliOperator::X},
                                {target_id,PauliOperator::X},
                        },.is_negative=false}}});
                if(cnot_correction_mode == CNOTCorrectionMode::ALWAYS)
                next_instructions.push({.operation={
                        SingleQubitOp{control_id, SingleQubitOp::Operator::Z}
                }});
        }
        else if(cnot_type == gates::CNOTType::ZX_WITH_MBM_TARGET_FIRST){
                next_instructions.push({.operation={
                        MultiPatchMeasurement{.observable={
                                {ancilla_id,PauliOperator::X},
                                {target_id,PauliOperator::X},
                        },.is_negative=false}}});
                if(cnot_correction_mode == CNOTCorrectionMode::ALWAYS)
                next_instructions.push({.operation={
                        SingleQubitOp{ancilla_id, SingleQubitOp::Operator::Z}
                }});
                next_instructions.push({.operation={
                        MultiPatchMeasurement{.observable={
                                {control_id, PauliOperator::Z},
                                {ancilla_id, PauliOperator::Z},
                        },.is_negative=false}}});
                if(cnot_correction_mode == CNOTCorrectionMode::ALWAYS)
                next_instructions.push({.operation={
                        SingleQubitOp{target_id, SingleQubitOp::Operator::X}
                }});
        }
        else LSTK_UNREACHABLE;

        next_instructions.push({.operation={SinglePatchMeasurement{ancilla_id, PauliOperator::X, false}}});

    }


    return next_instructions;
}

}