#include <lsqecc/ls_instructions/catalytic_s_gate_injection_stream.hpp>


namespace lsqecc
{

CatalyticSGateInjectionStream::CatalyticSGateInjectionStream(std::unique_ptr<LSInstructionStream> &&source,
                                                               IdGenerator& id_generator,
                                                               bool local_instructions)
 : source_(std::move(source)), id_generator_(id_generator), instruction_generator_(id_generator, local_instructions)
{}


bool CatalyticSGateInjectionStream::has_next_instruction() const
{
    return !next_instructions_.empty() || source_->has_next_instruction();
}


const tsl::ordered_set<PatchId> &CatalyticSGateInjectionStream::core_qubits() const
{
    return source_->core_qubits();
}

LSInstruction CatalyticSGateInjectionStream::get_next_instruction()
{
    if(!next_instructions_.empty()) return lstk::queue_pop(next_instructions_);

    if(!source_->has_next_instruction())
        throw std::logic_error{"CatalyticSGateInjectionStream: No more instructions from source"};

    LSInstruction new_instruction = source_->get_next_instruction();
    const auto* sgate = std::get_if<SingleQubitOp>(&new_instruction.operation);
    if( sgate && sgate->op == SingleQubitOp::Operator::S && core_qubits().contains(sgate->target))
    {
        const PatchId new_ancilla_id = id_generator_.new_id();

        // Find available Y state

        // Add gates
        auto instructions = instruction_generator_.make_cnot_instructions(
                                    sgate->target,
                                    ystate,
                                    gates::CNOTType::BELL_BASED,
                                    gates::CNOTAncillaPlacement::ANCILLA_FREE_PLACEMENT,
                                    CNOTCorrectionMode::NEVER);
        lstk::queue_extend(next_instructions_, instructions);

        next_instructions_.push({.operation={
                SingleQubitOp{
                    .target = ystate,
                    .op = SingleQubitOp::Operator::H}}});

        auto instructions = instruction_generator_.make_cnot_instructions(
                                    sgate->target,
                                    ystate,
                                    gates::CNOTType::BELL_BASED,
                                    gates::CNOTAncillaPlacement::ANCILLA_FREE_PLACEMENT,
                                    CNOTCorrectionMode::NEVER);
        lstk::queue_extend(next_instructions_, instructions);

        next_instructions_.push({.operation={
                SingleQubitOp{
                    .target = ystate,
                    .op = SingleQubitOp::Operator::H}}});
    }
    else
    {
        next_instructions_.push(new_instruction);
    }

    return lstk::queue_pop(next_instructions_);
}

}
