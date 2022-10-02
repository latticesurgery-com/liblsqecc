//
// Created by george on 26/09/22.
//

#include <lsqecc/ls_instructions/teleported_s_gate_injection_stream.hpp>


namespace lsqecc
{

TeleportedSGateInjectionStream::TeleportedSGateInjectionStream(std::unique_ptr<LSInstructionStream> &&source,
                                                               IdGenerator& id_generator)
 : source_(std::move(source)), id_generator_(id_generator)
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
    const auto* sgate = std::get_if<SingleQubitOp>(&new_instruction.operation);
    if( sgate && sgate->op == SingleQubitOp::Operator::S && core_qubits().contains(sgate->target))
    {
        const PatchId new_ancilla_id = id_generator_.new_id();

        next_instructions_.push({.operation={PatchInit{
                new_ancilla_id, PatchInit::InitializeableStates::Plus}}});
        // S-Gate distillation
        next_instructions_.push({.operation={
                SingleQubitOp{
                    .target = new_ancilla_id,
                    .op = SingleQubitOp::Operator::S}}});
        // Teleport the state
        next_instructions_.push({.operation={
                MultiPatchMeasurement{.observable={
                        {new_ancilla_id, PauliOperator::Z},
                        {sgate->target, PauliOperator::Z},
                },.is_negative=false}}});
        next_instructions_.push({.operation={SinglePatchMeasurement{new_ancilla_id, PauliOperator::Z, false}}});
    }
    else
    {
        next_instructions_.push(new_instruction);
    }

    return lstk::queue_pop(next_instructions_);
}

}
