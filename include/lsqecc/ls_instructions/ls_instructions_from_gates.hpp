#ifndef LSQECC_LS_INSTRUCTIONS_FROM_GATES_HPP
#define LSQECC_LS_INSTRUCTIONS_FROM_GATES_HPP

#include <lsqecc/patches/patches.hpp>
#include <lsqecc/gates/gates.hpp>
#include <lsqecc/ls_instructions/ls_instructions.hpp>
#include <lsqecc/ls_instructions/id_generator.hpp>

namespace lsqecc
{

enum class CNOTCorrectionMode {
    NEVER,
    ALWAYS
    // TODO add RANDOMIZED
};

class LSIinstructionFromGatesGenerator
{
public:
    explicit LSIinstructionFromGatesGenerator(IdGenerator& id_generator, bool local_instructions);

    std::queue<LSInstruction> make_t_gate_instructions(PatchId target_id);
    std::queue<LSInstruction> make_cnot_instructions(
            PatchId control_id,
            PatchId target_id,
            gates::CNOTType cnot_type,
            gates::CNOTAncillaPlacement cnot_ancilla_placement,
            CNOTCorrectionMode cnot_correction_mode);

private:
    IdGenerator& id_generator_;
    bool local_instructions_;
};

}


#endif //LSQECC_LS_INSTRUCTIONS_FROM_GATES_HPP
