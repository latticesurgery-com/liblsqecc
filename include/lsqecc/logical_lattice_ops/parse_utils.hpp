#ifndef LSQECC_PARSE_UTILS_HPP
#define LSQECC_PARSE_UTILS_HPP

#include <lsqecc/logical_lattice_ops/logical_lattice_ops.hpp>

namespace lsqecc {

std::vector<std::string_view> sv_split_on_char(std::string_view source, char c);

PatchId parse_patch_id(const std::string_view& input);

PatchInit::InitializeableStates parse_init_state(std::string_view);

}

#endif //LSQECC_PARSE_UTILS_HPP
