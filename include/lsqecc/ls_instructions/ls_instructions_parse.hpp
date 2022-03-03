#ifndef LSQECC_LS_INSTRUCTIONS_PARSE_HPP
#define LSQECC_LS_INSTRUCTIONS_PARSE_HPP


#include <lsqecc/ls_instructions/ls_instructions.hpp>


namespace lsqecc {

InMemoryLogicalLatticeComputation parse_ls_instructions(std::string_view source);

}

#endif //LSQECC_LS_INSTRUCTIONS_PARSE_HPP
