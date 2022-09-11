#ifndef LSQECC_LS_INSTRUCTIONS_FROM_GATES_HPP
#define LSQECC_LS_INSTRUCTIONS_FROM_GATES_HPP

#include <lsqecc/patches/patches.hpp>
#include <lsqecc/gates/gates.hpp>
#include <lsqecc/ls_instructions/ls_instructions.hpp>

namespace lsqecc
{


class LSIinstructionFromGatesGenerator
{
public:
    explicit LSIinstructionFromGatesGenerator(PatchId ancilla_state_id_start);

    std::queue<LSInstruction> make_t_gate_instructions(PatchId target_id);
    std::queue<LSInstruction> make_cnot_instructions(
            PatchId control_id, PatchId target_id, gates::CNOTType cnot_type, gates::CNOTAncillaPlacement cnot_ancilla_placement);

private:
    PatchId get_next_ancilla_state_id();
    PatchId ancilla_state_id_counter_;
};

}


#endif //LSQECC_LS_INSTRUCTIONS_FROM_GATES_HPP
