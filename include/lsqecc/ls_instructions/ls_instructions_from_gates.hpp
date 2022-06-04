#ifndef LSQECC_LS_INSTRUCTIONS_FROM_GATES_HPP
#define LSQECC_LS_INSTRUCTIONS_FROM_GATES_HPP

#include <lsqecc/patches/patches.hpp>
#include <lsqecc/ls_instructions/ls_instructions.hpp>

namespace lsqecc
{

class LSIinstructionFromGatesGenerator
{
public:
    explicit LSIinstructionFromGatesGenerator(PatchId magic_state_id_start);
private:
    PatchId get_next_magic_state_id();

    std::queue<LSInstruction> make_t_gate_instructions(PatchId target_id);
    std::queue<LSInstruction> make_cnot_instructions(PatchId source_id, PatchId target_id);

    PatchId magic_state_id_counter_;
};

}


#endif //LSQECC_LS_INSTRUCTIONS_FROM_GATES_HPP
