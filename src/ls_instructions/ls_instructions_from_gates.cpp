#include <lsqecc/ls_instructions/ls_instructions_from_gates.hpp>

namespace lsqecc
{


LSIinstructionFromGatesGenerator::LSIinstructionFromGatesGenerator(PatchId magic_state_id_start)
: magic_state_id_counter_(magic_state_id_start)
{}

PatchId LSIinstructionFromGatesGenerator::get_next_magic_state_id()
{
    return magic_state_id_counter_++;
}

std::queue<LSInstruction> LSIinstructionFromGatesGenerator::make_t_gate_instructions(PatchId target_id)
{
    std::queue<LSInstruction>   next_instructions;
    PatchId new_magic_state_id = get_next_magic_state_id();
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

}