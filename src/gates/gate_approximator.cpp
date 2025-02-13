#include <lsqecc/gates/gate_approximator.hpp>

#include <algorithm>


bool is_power_of_two(lsqecc::ArbitraryPrecisionInteger n)
{
    if(n==1)
        return true;
    return (n & (n - 1)) == 0 && n != 0;
}



#ifdef USE_GRIDSYNTH

#include <HsFFI.h>
#include <gridsynth_ccals.h>

namespace lsqecc {

using namespace gates;


/// Singleton instance to use RAII to manage Haskell's runtime because:
/// `hs_init_ghc: reinitializing the RTS after shutdown is not currently supported`
struct HaskellRuntime {

    static void require_init()
    {
        if(!instance)
            instance = std::make_unique<HaskellRuntime>();
    }

    HaskellRuntime()
    {
        std::string prog_name = "lsqecc_simulated_prog_name";
        std::vector<char*> arg_list;
        arg_list.push_back(const_cast<char*>(prog_name.c_str()));
        int argc = arg_list.size();
        char** argv = arg_list.data();
        hs_init(&argc, &argv);
    }

    ~HaskellRuntime()
    {
        hs_exit();
    }

    inline static std::unique_ptr<HaskellRuntime> instance;

};



std::vector<char> do_gridsynth_call(double precision, const std::string& angle)
{
    std::vector<char> buffer(2048, '\0');

    HaskellRuntime::require_init();
    const int result = gridsynth_angle_to_seq_with_precision_hs(precision, angle.c_str(), buffer.data(), buffer.size());
    
    if(result != 0)
        throw std::logic_error{lstk::cat("Call to Gridsynth C Call returned ", result)};

    return buffer;
}


Gate gate_from_name(char name, QubitNum target_qubit)
{
    switch (name)
    {
        case 'X': return X(target_qubit);
        case 'Z': return Z(target_qubit);
        case 'S': return S(target_qubit);
        case 'T': return T(target_qubit);
        case 'H': return H(target_qubit);
    }
    throw std::runtime_error{lstk::cat("Unsupported gate form Gridsynth: ", name)};
}

std::vector<Gate> approximate_RZ_gate_gridsynth(const RZ rz_gate, double rz_precision_log_ten_negative)
{
    std::vector<Gate> out;

    Fraction pi_fraction = rz_gate.pi_fraction;
    std::string angle = (pi_fraction.is_negative? "-":"+")
                        + std::to_string(pi_fraction.num) 
                        + "*pi/" 
                        + std::to_string(pi_fraction.den);
    std::vector<char> gate_names{do_gridsynth_call(rz_precision_log_ten_negative, angle)};
    for (auto gate_name = gate_names.begin(); gate_name != gate_names.end(); gate_name++)
    {
        if(*gate_name == '\0')
            break;
        
        if(*gate_name != 'W') // Skip phases
            out.push_back(gate_from_name(*gate_name, rz_gate.target_qubit));
    }

    std::reverse(out.begin(), out.end());

    return out;
}

}


#endif // USE_GRIDSYNTH


namespace lsqecc
{

using namespace gates;



std::vector<Gate> approximate_RZ_gate_cached(const RZ rz_gate)
{
    std::vector<Gate> res;
    if ((rz_gate.pi_fraction.num == 1 || rz_gate.pi_fraction.num == -1) && is_power_of_two(rz_gate.pi_fraction.den))
    {
        // TODO Do the approximation
        throw std::runtime_error{lstk::cat("Not implemented! No cached decompositions. We are working on this...")};
    }
    throw std::runtime_error{lstk::cat("Can only approximate pi/2^n phase gates in cached mode, got rz(", print_pi_fraction(rz_gate.pi_fraction),")\n"
                                       "If you really need non pi/2^n gates consider enabling the gridsynth integration"
                                       )};

}


std::vector<Gate> approximate_RZ_gate(const RZ rz_gate, double rz_precision_log_ten_negative)
{
#ifdef USE_GRIDSYNTH
    return approximate_RZ_gate_gridsynth(rz_gate, rz_precision_log_ten_negative);
#else
    return approximate_RZ_gate_cached(rz_gate);
#endif // USE_GRIDSYNTH
}

}


