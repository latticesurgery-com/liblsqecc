#include <lsqecc/gates/gate_approximator.hpp>


#include <gtest/gtest.h>

#include <lsqecc/gates/gate_approximator.hpp>

#include <sstream>

using namespace lsqecc;

#ifdef USE_GRIDSYNTH

TEST(gate_approximator, approximate_RZ_gate)
{
    std::vector<gates::Gate> res = approximate_RZ_gate(gates::RZ{1,Fraction{1,8}},10);
    std::stringstream ss;
    for (const auto& g : res)
    {
        ss << g << std::endl;
    }

    ASSERT_EQ("s qreg[1];\nx qreg[1];\nh qreg[1];\ns qreg[1];\nt qreg[1];\nh qreg[1];\nt qreg[1];"
            "\nh qreg[1];\ns qreg[1];\nt qreg[1];\nh qreg[1];\ns qreg[1];\nt qreg[1];\nh qreg[1];\ns "
            "qreg[1];\nt qreg[1];\nh qreg[1];\nt qreg[1];\nh qreg[1];\ns qreg[1];\nt qreg[1];\nh qreg[1];"
            "\ns qreg[1];\nt qreg[1];\nh qreg[1];\nt qreg[1];\nh qreg[1];\ns qreg[1];\nt qreg[1];\nh qreg[1];"
            "\nt qreg[1];\nh qreg[1];\ns qreg[1];\nt qreg[1];\nh qreg[1];\ns qreg[1];\nt qreg[1];\nh qreg[1];"
            "\ns qreg[1];\nt qreg[1];\nh qreg[1];\nt qreg[1];\nh qreg[1];\ns qreg[1];\nt qreg[1];\nh qreg[1];"
            "\ns qreg[1];\nt qreg[1];\nh qreg[1];\nt qreg[1];\nh qreg[1];\ns qreg[1];\nt qreg[1];\nh qreg[1];"
            "\nt qreg[1];\nh qreg[1];\ns qreg[1];\nt qreg[1];\nh qreg[1];\ns qreg[1];\nt qreg[1];\nh qreg[1];",
        ss.str()
    );

}

#else

#endif // USE_GRIDSYNTH

