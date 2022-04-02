# include <lsqecc/gates/gates.hpp>
# include <lsqecc/gates/pi_over_2_to_the_n_rz_gate_approximations.hpp>

#include <array>

namespace lsqecc {



bool is_power_of_two(ArbitraryPrecisionInteger n)
{
    return (n & (n - 1)) == 0 && n != 0;
}



std::vector<gates::Gate> approximate_RZ_gate(const gates::RZ rz_gate)
{
    std::vector<gates::Gate> res;
    if((rz_gate.pi_fraction.num == 1 || rz_gate.pi_fraction.num == -1) && is_power_of_two(rz_gate.pi_fraction.den))
    {
        // Do the approximation
    }
    throw std::runtime_error{lstk::cat("Can only approximate pi/2^n phase gates, got rz(pi*",
            rz_gate.pi_fraction.num, "/", rz_gate.pi_fraction.num, ") ")};

}



std::vector<gates::Gate> decompose_CRZ_gate(const gates::ControlledGate& crz_gate)
{
    const auto& rz_gate = std::get<gates::RZ>(crz_gate.target_gate);

    /*
     * Use the follwing identity:
     *                                 ┌───────────┐
     * q_0: ─────■─────           q_0: ┤ Rz(π/(2n))├──■──────────────────■──
     *      ┌────┴────┐     =          ├───────────┤┌─┴─┐┌────────────┐┌─┴─┐
     * q_1: ┤ Rz(π/n) ├           q_1: ┤ Rz(π/(2n))├┤ X ├┤ Rz(-π/(2n))├┤ X ├
     *      └─────────┘                └───────────┘└───┘└────────────┘└───┘
     */

    std::vector<gates::Gate> res;
    lstk::vector_extend(res, to_clifford_plus_t(
            gates::RZ{crz_gate.control_qubit,Fraction{rz_gate.pi_fraction.num,
                                                      rz_gate.pi_fraction.den*2}}));
    lstk::vector_extend(res, to_clifford_plus_t(
            gates::RZ{rz_gate.target_qubit,Fraction{rz_gate.pi_fraction.num,
                                                    rz_gate.pi_fraction.den*2}}));
    res.emplace_back(gates::CNOT(rz_gate.target_qubit, crz_gate.control_qubit));
    lstk::vector_extend(res, to_clifford_plus_t(
            gates::RZ{rz_gate.target_qubit,Fraction{-rz_gate.pi_fraction.num,
                                                    rz_gate.pi_fraction.den*2}}));
    res.emplace_back(gates::CNOT(rz_gate.target_qubit, crz_gate.control_qubit));

}


std::vector<gates::Gate> to_clifford_plus_t(const gates::Gate& gate)
{
    if(const auto* basic_gate = std::get_if<gates::BasicGate>(&gate))
        return {*basic_gate};
    else if (const auto* rz_gate = std::get_if<gates::RZ>(&gate))
        throw std::runtime_error{"Not implemented: rz to clifford+T"};
    else {
        const auto controlled_gate = std::get<gates::ControlledGate>(gate);
        if(std::holds_alternative<gates::RZ>(controlled_gate.target_gate))
            return decompose_CRZ_gate(controlled_gate);

        throw std::runtime_error{"Not implemented: non crz controlled gate to clifford + T"};
    }
}

}

