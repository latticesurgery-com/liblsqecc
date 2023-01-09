#ifndef LSQECC_GATE_APPROXIMATOR_HPP
#define LSQECC_GATE_APPROXIMATOR_HPP

#include <lsqecc/gates/gates.hpp>

namespace lsqecc 
{


std::vector<gates::Gate> approximate_RZ_gate(const gates::RZ rz_gate, double rz_precision_log_ten_negative);

}

#endif
