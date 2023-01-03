#include <lsqecc/gates/gate_approximator.hpp>


#include <HsFFI.h>
#include <gridsynth_ccals.h>

namespace lsqecc
{

using namespace gates;

bool is_power_of_two(ArbitraryPrecisionInteger n)
{
    return (n & (n - 1)) == 0 && n != 0;
}




std::vector<Gate> approximate_RZ_gate_gridsynth(const RZ rz_gate)
{
    return {};
}


std::vector<Gate> approximate_RZ_gate_cached(const RZ rz_gate)
{
    std::vector<Gate> res;
    if ((rz_gate.pi_fraction.num == 1 || rz_gate.pi_fraction.num == -1) && is_power_of_two(rz_gate.pi_fraction.den))
    {
        // TODO Do the approximation
        LSTK_NOT_IMPLEMENTED;
    }
    throw std::runtime_error{lstk::cat("Can only approximate pi/2^n phase gates in cached mode, got rz(pi*",
                                       rz_gate.pi_fraction.num, "/", rz_gate.pi_fraction.num, ")\n"
                                       "If you really need non pi/2^n gates consider enabling the gridsynth integration"
                                       )};

}


std::vector<Gate> approximate_RZ_gate(const RZ rz_gate)
{
#ifdef USE_GRIDSYNTH
    return approximate_RZ_gate_gridsynth(rz_gate);
#else
    return approximate_RZ_gate_cached(rz_gate);
#endif // USE_GRIDSYNTH
}


}