
#include <sstream>

#include <gtest/gtest.h>

#include <lsqecc/gates/gate_approximator.hpp>


using namespace lsqecc;

#ifdef USE_GRIDSYNTH

TEST(gate_approximator, approximate_RZ_gate)
{
    std::vector<gates::Gate> res = approximate_RZ_gate(gates::RZ{1,Fraction{1,16}},10);
    std::stringstream ss;
    for (const auto& g : res)
    {
        ss << g << std::endl;
    }

    ASSERT_EQ("s q[1];\nx q[1];\nh q[1];\ns q[1];\nt q[1];\nh q[1];\nt q[1];"
            "\nh q[1];\ns q[1];\nt q[1];\nh q[1];\ns q[1];\nt q[1];\nh q[1];\ns "
            "q[1];\nt q[1];\nh q[1];\nt q[1];\nh q[1];\ns q[1];\nt q[1];\nh q[1];"
            "\ns q[1];\nt q[1];\nh q[1];\nt q[1];\nh q[1];\ns q[1];\nt q[1];\nh q[1];"
            "\nt q[1];\nh q[1];\ns q[1];\nt q[1];\nh q[1];\ns q[1];\nt q[1];\nh q[1];"
            "\ns q[1];\nt q[1];\nh q[1];\nt q[1];\nh q[1];\ns q[1];\nt q[1];\nh q[1];"
            "\ns q[1];\nt q[1];\nh q[1];\nt q[1];\nh q[1];\ns q[1];\nt q[1];\nh q[1];"
            "\nt q[1];\nh q[1];\ns q[1];\nt q[1];\nh q[1];\ns q[1];\nt q[1];\nh q[1];\n",
        ss.str()
    );

}

TEST(gate_approximator, approximate_RZ_gate_minus_3_pi_over_2)
{
    std::vector<gates::Gate> res = approximate_RZ_gate(gates::RZ{1,Fraction{3,2,/*is_negative=*/true}},10);
    std::stringstream ss;
    for (const auto& g : res)
    {
        ss << g << std::endl;
    }

    ASSERT_EQ("s q[1];\n",
        ss.str()
    );

}

#else

#endif // USE_GRIDSYNTH

