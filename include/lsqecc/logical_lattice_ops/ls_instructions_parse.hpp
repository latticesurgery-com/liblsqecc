#ifndef LSQECC_LS_INSTRUCTIONS_PARSE_HPP
#define LSQECC_LS_INSTRUCTIONS_PARSE_HPP


#include <lsqecc/logical_lattice_ops/logical_lattice_ops.hpp>


namespace lsqecc {

std::vector<LogicalLatticeOperation> parse_ls_instructions(std::string_view source);

}

#endif //LSQECC_LS_INSTRUCTIONS_PARSE_HPP
