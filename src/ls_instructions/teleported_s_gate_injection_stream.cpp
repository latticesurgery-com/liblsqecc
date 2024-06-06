//
// Created by george on 26/09/22.
//

#include <lsqecc/ls_instructions/teleported_s_gate_injection_stream.hpp>


namespace lsqecc
{

TeleportedSGateInjectionStream::TeleportedSGateInjectionStream(std::unique_ptr<LSInstructionStream> &&source,
                                                               IdGenerator& id_generator,
                                                               bool always_rotate)
 : source_(std::move(source)), id_generator_(id_generator), always_rotate_(always_rotate)
{}


bool TeleportedSGateInjectionStream::has_next_instruction() const
{
    return !next_instructions_.empty() || source_->has_next_instruction();
}


const tsl::ordered_set<PatchId> &TeleportedSGateInjectionStream::core_qubits() const
{
    return source_->core_qubits();
}

LSInstruction TeleportedSGateInjectionStream::get_next_instruction()
{
    if(!next_instructions_.empty()) return lstk::queue_pop(next_instructions_);

    if(!source_->has_next_instruction())
        throw std::logic_error{"TeleportedSGateInjectionStream: No more instructions from source"};

    LSInstruction new_instruction = source_->get_next_instruction();
    const auto* gate = std::get_if<SingleQubitOp>(&new_instruction.operation);
    if( gate && gate->op == SingleQubitOp::Operator::S && core_qubits().contains(gate->target))
    {
        const PatchId ystate_id = id_generator_.new_id();

        // Request Y state
        next_instructions_.push({.operation={YStateRequest{ystate_id, gate->target}}, .wait_at_most_for=YStateRequest::DEFAULT_WAIT, .clients={gate->target}});

        // Teleport the state
        next_instructions_.push({.operation={
                MultiPatchMeasurement{.observable={
                        {ystate_id, PauliOperator::Z},
                        {gate->target, PauliOperator::Z},
                },.is_negative=false}}});
        next_instructions_.push({.operation={SinglePatchMeasurement{ystate_id, PauliOperator::X, false}}});

        // This Pauli Z is applied probabilistically according to the XOR of the two measurement outcomes.
        next_instructions_.push({.operation={SingleQubitOp{gate->target,SingleQubitOp::Operator::Z}}});
    }
    else if (always_rotate_ && gate 
            && gate->op == SingleQubitOp::Operator::H && core_qubits().contains(gate->target))
    {
        next_instructions_.push(new_instruction);
        next_instructions_.push({RotateSingleCellPatch{gate->target}});
    }
    else
    {
        next_instructions_.push(new_instruction);
    }

    return lstk::queue_pop(next_instructions_);
}

}
