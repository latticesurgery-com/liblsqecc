#ifndef LSQECC_PARSE_GATES_HPP
#define LSQECC_PARSE_GATES_HPP

#include <lsqecc/gates/gates.hpp>

#include <string_view>
#include <istream>
#include <vector>

namespace lsqecc {


std::vector<gates::Gate> parse_all_gates_from_qasm(std::istream istream);

}



#endif //LSQECC_PARSE_GATES_HPP
