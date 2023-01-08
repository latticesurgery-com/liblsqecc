#include <lsqecc/gates/gate_approximator.hpp>


#include <gtest/gtest.h>

#include <lsqecc/gates/gate_approximator.hpp>

using namespace lsqecc;

TEST(gate_approximator, approximate_RZ_gate)
{
    std::vector<gates::Gate> res = approximate_RZ_gate(gates::RZ{1,Fraction{1,8}});
    std::cout << "seq:" << std::endl;
    for (const auto& g : res)
    {
        std::cout << g << std::endl;
    }

}
