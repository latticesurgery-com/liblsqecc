#include <lsqecc/ls_instructions/ls_instructions_from_gates.hpp>

namespace lsqecc
{

LSIinstructionFromGatesGenerator::LSIinstructionFromGatesGenerator(IdGenerator& id_generator, bool local_instructions)
: id_generator_(id_generator), local_instructions_(local_instructions)
{}

std::queue<LSInstruction> LSIinstructionFromGatesGenerator::make_t_gate_instructions(PatchId target_id)
{
    std::queue<LSInstruction> next_instructions;
    PatchId new_magic_state_id = id_generator_.new_id();
    next_instructions.push({.operation={MagicStateRequest{new_magic_state_id}}, .wait_at_most_for=MagicStateRequest::DEFAULT_WAIT});
    if (local_instructions_)
    {
        auto instructions = make_cnot_instructions(
                            target_id,
                            new_magic_state_id,
                            gates::CNOTType::BELL_BASED,
                            gates::CNOTAncillaPlacement::ANCILLA_FREE_PLACEMENT,
                            CNOTCorrectionMode::NEVER);
        lstk::queue_extend(next_instructions, instructions);
        next_instructions.push({.operation={SinglePatchMeasurement{new_magic_state_id, PauliOperator::Z, false}}});

        next_instructions.push({.operation={SingleQubitOp{target_id, SingleQubitOp::Operator::S}}});
    }
    else 
    {
        next_instructions.push({.operation={
                MultiPatchMeasurement{.observable={
                        {target_id,PauliOperator::Z},
                        {new_magic_state_id,PauliOperator::Z},
                },.is_negative=false}}});
        next_instructions.push({.operation={SinglePatchMeasurement{
                new_magic_state_id, PauliOperator::X, false
        }}});
        next_instructions.push({.operation={SingleQubitOp{target_id, SingleQubitOp::Operator::S}}});
    }

    return next_instructions;
}

std::queue<LSInstruction> LSIinstructionFromGatesGenerator::make_cnot_instructions(
        PatchId control_id,
        PatchId target_id,
        gates::CNOTType cnot_type,
        gates::CNOTAncillaPlacement cnot_ancilla_placement,
        CNOTCorrectionMode cnot_correction_mode)
{
    std::queue<LSInstruction> next_instructions;

    if (cnot_type == gates::CNOTType::BELL_BASED) {
        next_instructions.push({.operation={BellBasedCNOT{control_id, target_id}}});

        // PatchId id1 = id_generator_.new_id();
        // PatchId id2 = id_generator_.new_id();
        // next_instructions.push({.operation={
        //         BellPairInit{id1, id2, PlaceNexTo{control_id, PauliOperator::Z}, PlaceNexTo{target_id, PauliOperator::X}}}});
        // next_instructions.push({.operation={
        //         MultiPatchMeasurement{.observable={
        //                 {control_id, PauliOperator::Z},
        //                 {id1, PauliOperator::Z},
        //         },.is_negative=false}}});
        // next_instructions.push({.operation={
        //         MultiPatchMeasurement{.observable={
        //                 {id2,PauliOperator::X},
        //                 {target_id,PauliOperator::X},
        //         },.is_negative=false}}});
        // next_instructions.push({.operation={SinglePatchMeasurement{id1, PauliOperator::X, false}}});
        // next_instructions.push({.operation={SinglePatchMeasurement{id2, PauliOperator::Z, false}}});

        // if(cnot_correction_mode == CNOTCorrectionMode::ALWAYS) {
        //         next_instructions.push({.operation={
        //                 SingleQubitOp{control_id, SingleQubitOp::Operator::Z}}});
        //         next_instructions.push({.operation={
        //                 SingleQubitOp{target_id, SingleQubitOp::Operator::X}}});
        // }
    }
    
    else {

        // Control is green -> smooth -> measures Z otimes Z
        // Target is red -> rough -> measures X otimes X

        std::optional<PlaceNexTo> place_ancilla_next_to;
        if(cnot_ancilla_placement == gates::CNOTAncillaPlacement::ANCILLA_NEXT_TO_CONTROL)
                place_ancilla_next_to = PlaceNexTo{control_id, PauliOperator::Z}; 
        else if(cnot_ancilla_placement == gates::CNOTAncillaPlacement::ANCILLA_NEXT_TO_TARGET)
                place_ancilla_next_to = PlaceNexTo{target_id, PauliOperator::X};

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