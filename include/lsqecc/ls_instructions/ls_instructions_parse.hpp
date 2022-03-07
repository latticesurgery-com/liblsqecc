#ifndef LSQECC_LS_INSTRUCTIONS_PARSE_HPP
#define LSQECC_LS_INSTRUCTIONS_PARSE_HPP


#include <lsqecc/ls_instructions/ls_instructions.hpp>


namespace lsqecc {

struct InstructionParseException : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

LSInstruction parse_ls_instruction(std::string_view line);

InMemoryLogicalLatticeComputation parse_ls_instructions(std::string_view source);

}

#endif //LSQECC_LS_INSTRUCTIONS_PARSE_HPP
