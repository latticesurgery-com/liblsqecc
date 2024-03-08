#include <sstream>

#include <gtest/gtest.h>

#include <lsqecc/gates/parse_gates.hpp>

using namespace lsqecc;

TEST(parse_gates, rz_pi_over_1)
{
    std::string gate = "rz(pi/1) qreg[22];";
    ParseGateResult res = parse_gate(gate);
    ASSERT_TRUE(std::holds_alternative<gates::Gate>(res));
    auto g = std::get<gates::Gate>(res);
    ASSERT_TRUE(std::holds_alternative<gates::BasicSingleQubitGate>(g));
    auto rz = std::get<gates::BasicSingleQubitGate>(g);
    ASSERT_EQ(22, rz.target_qubit);
    ASSERT_EQ(gates::BasicSingleQubitGate::Type::Z, rz.gate_type);
}

TEST(parse_gates, rz_2_pi_over_7)
{
    std::string gate = "rz(2*pi/7) qreg[22];";
    ParseGateResult res = parse_gate(gate);
    ASSERT_TRUE(std::holds_alternative<gates::Gate>(res));
    auto g = std::get<gates::Gate>(res);
    ASSERT_TRUE(std::holds_alternative<gates::RZ>(g));
    auto rz = std::get<gates::RZ>(g);
    ASSERT_EQ(22, rz.target_qubit);
    ASSERT_EQ(2, rz.pi_fraction.num);
    ASSERT_EQ(7, rz.pi_fraction.den);
}