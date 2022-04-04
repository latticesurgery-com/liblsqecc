#ifndef LSQECC_LS_INSTRUCTION_FROM_GATES_HPP
#define LSQECC_LS_INSTRUCTION_FROM_GATES_HPP


#include <lsqecc/gates/gates.hpp>
#include <lsqecc/ls_instructions/ls_instructions.hpp>

namespace lsqecc {

std::vector<LSInstruction> from_gate(const gates::Gate& gate);

}

#endif // LSQECC_LS_INSTRUCTION_FROM_GATES_HPP