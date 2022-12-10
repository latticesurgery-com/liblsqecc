# include <lsqecc/gates/gates.hpp>
# include <lsqecc/gates/pi_over_2_to_the_n_rz_gate_approximations.hpp>

#include <array>

namespace lsqecc::gates {


bool is_power_of_two(ArbitraryPrecisionInteger n)
{
    return (n & (n - 1)) == 0 && n != 0;
}


std::vector<Gate> approximate_RZ_gate(const RZ rz_gate)
{
    std::vector<Gate> res;
    if ((rz_gate.pi_fraction.num == 1 || rz_gate.pi_fraction.num == -1) && is_power_of_two(rz_gate.pi_fraction.den))
    {
        // TODO Do the approximation
    }
    throw std::runtime_error{lstk::cat("Can only approximate pi/2^n phase gates, got rz(pi*",
                                       rz_gate.pi_fraction.num, "/", rz_gate.pi_fraction.num, ") ")};

}


std::vector<Gate> decompose_CRZ_gate(const ControlledGate &crz_gate)
{
    const auto &rz_gate = std::get<RZ>(crz_gate.target_gate);
    /*
     * Use the following identity:
     *                                 ┌───────────┐
     * q_0: ─────■─────           q_0: ┤ Rz(π/(2n))├──■──────────────────■──
     *      ┌────┴────┐     =          ├───────────┤┌─┴─┐┌────────────┐┌─┴─┐
     * q_1: ┤ Rz(π/n) ├           q_1: ┤ Rz(π/(2n))├┤ X ├┤ Rz(-π/(2n))├┤ X ├
     *      └─────────┘                └───────────┘└───┘└────────────┘└───┘
     */

    std::vector<Gate> res;
    lstk::vector_extend(res, to_clifford_plus_t(
            RZ{crz_gate.control_qubit, Fraction{rz_gate.pi_fraction.num,
                                                rz_gate.pi_fraction.den * 2}}));
    lstk::vector_extend(res, to_clifford_plus_t(
            RZ{rz_gate.target_qubit, Fraction{rz_gate.pi_fraction.num,
                                              rz_gate.pi_fraction.den * 2}}));
    res.emplace_back(
            CNOT(rz_gate.target_qubit, crz_gate.control_qubit, crz_gate.cnot_type, crz_gate.cnot_ancilla_placement));
    lstk::vector_extend(res, to_clifford_plus_t(
            RZ{rz_gate.target_qubit, Fraction{-rz_gate.pi_fraction.num,
                                              rz_gate.pi_fraction.den * 2}}));
    res.emplace_back(
            CNOT(rz_gate.target_qubit, crz_gate.control_qubit, crz_gate.cnot_type, crz_gate.cnot_ancilla_placement));

    return res;
}


std::vector<Gate> to_clifford_plus_t(const Gate &gate)
{
    if (const auto *basic_gate = std::get_if<BasicSingleQubitGate>(&gate))
        return {*basic_gate};
    else if (std::holds_alternative<RZ>(gate))
        throw std::runtime_error{"Not implemented: rz to clifford+T"};
    else
    {
        const auto controlled_gate = std::get<ControlledGate>(gate);
        if (std::holds_alternative<RZ>(controlled_gate.target_gate))
            return decompose_CRZ_gate(controlled_gate);

        throw std::runtime_error{"Not implemented: non crz controlled gate to clifford + T"};
    }
}

std::string_view CNOTType_toString(CNOTType cnot_type)
{
    using namespace std::string_view_literals;
    switch (cnot_type)
    {
        case CNOTType::ZX_WITH_MBM_CONTROL_FIRST:
            return "ZXWithMBMControlFirst"sv;
        case CNOTType::ZX_WITH_MBM_TARGET_FIRST:
            return "ZXWithMBMTargetFirst"sv;
    }
    LSTK_UNREACHABLE;
}


std::optional<CNOTType> CNOTType_fromString(std::string_view s)
{
    if (s == CNOTType_toString(CNOTType::ZX_WITH_MBM_CONTROL_FIRST)) return CNOTType::ZX_WITH_MBM_CONTROL_FIRST;
    if (s == CNOTType_toString(CNOTType::ZX_WITH_MBM_TARGET_FIRST)) return CNOTType::ZX_WITH_MBM_TARGET_FIRST;
    else return std::nullopt;
}


std::string_view CNOTAncillaPlacement_toString(CNOTAncillaPlacement v)
{
    using namespace std::string_view_literals;
    switch (v)
    {
        case CNOTAncillaPlacement::ANCILLA_FREE_PLACEMENT:
            return "AncillaFreePlacement"sv;
        case CNOTAncillaPlacement::ANCILLA_NEXT_TO_CONTROL:
            return "AncillaNextToControl"sv;
        case CNOTAncillaPlacement::ANCILLA_NEXT_TO_TARGET:
            return "AncillaNextToTarget"sv;
    }
    LSTK_UNREACHABLE;
}

std::optional<CNOTAncillaPlacement> CNOTAncillaPlacement_fromString(std::string_view s)
{
    if (s == CNOTAncillaPlacement_toString(CNOTAncillaPlacement::ANCILLA_FREE_PLACEMENT))
        return CNOTAncillaPlacement::ANCILLA_FREE_PLACEMENT;
    if (s == CNOTAncillaPlacement_toString(CNOTAncillaPlacement::ANCILLA_NEXT_TO_CONTROL))
        return CNOTAncillaPlacement::ANCILLA_NEXT_TO_CONTROL;
    if (s == CNOTAncillaPlacement_toString(CNOTAncillaPlacement::ANCILLA_NEXT_TO_TARGET))
        return CNOTAncillaPlacement::ANCILLA_NEXT_TO_TARGET;
    else return std::nullopt;
}

QubitNum get_target_gate(const Gate& gate){

    if (const auto* basic_single_qubit_gate = std::get_if<gates::BasicSingleQubitGate>(&gate))
    {
        return basic_single_qubit_gate->target_qubit;
    } else if (const auto* rz_gate = std::get_if<gates::RZ>(&gate))
    {
        return rz_gate->target_qubit;
    } else if (const auto* controlled_gate = std::get_if<gates::ControlledGate>(&gate))
    {
        if (const auto* inner_basic_single_qubit_gate = std::get_if<gates::BasicSingleQubitGate>(&controlled_gate->target_gate))
            return get_target_gate(Gate{*inner_basic_single_qubit_gate});
        else if (const auto* inner_rz_gate = std::get_if<gates::RZ>(&controlled_gate->target_gate))
            return get_target_gate(Gate{*inner_rz_gate});
    }
    
    LSTK_UNREACHABLE;
}


}

