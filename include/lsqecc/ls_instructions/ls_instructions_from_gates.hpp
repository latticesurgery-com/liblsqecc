#ifndef LSQECC_LS_INSTRUCTIONS_FROM_GATES_HPP
#define LSQECC_LS_INSTRUCTIONS_FROM_GATES_HPP

#include <lsqecc/patches/patches.hpp>
#include <lsqecc/ls_instructions/ls_instructions.hpp>

namespace lsqecc
{

enum class CNOTType
{
    ZX_WITH_MBM_CONTROL_FIRST,
    ZX_WITH_MBM_TARGET_FIRST
};

class LSIinstructionFromGatesGenerator
{
public:
    explicit LSIinstructionFromGatesGenerator(PatchId ancilla_state_id_start);

    std::queue<LSInstruction> make_t_gate_instructions(PatchId target_id);
    std::queue<LSInstruction> make_cnot_instructions(PatchId control_id, PatchId target_id, CNOTType cnot_type);

private:
    PatchId get_next_ancilla_state_id();
    PatchId ancilla_state_id_counter_;
};

}


#endif //LSQECC_LS_INSTRUCTIONS_FROM_GATES_HPP
