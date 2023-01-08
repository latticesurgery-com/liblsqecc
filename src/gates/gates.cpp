# include <lsqecc/gates/gates.hpp>
# include <lsqecc/gates/pi_over_2_to_the_n_rz_gate_approximations.hpp>
# include <lsqecc/gates/gate_approximator.hpp>

#include <array>

namespace lsqecc::gates {


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
    else if (const auto *rz_gate = std::get_if<RZ>(&gate))
        return approximate_RZ_gate(*rz_gate);
    else
    {
        const auto controlled_gate = std::get<ControlledGate>(gate);
        if (std::holds_alternative<RZ>(controlled_gate.target_gate))
            return decompose_CRZ_gate(controlled_gate);
        else if (std::holds_alternative<BasicSingleQubitGate>(controlled_gate.target_gate))
            return {gate};
        else
            LSTK_NOT_IMPLEMENTED;
    }
}


bool is_clifford_plus_t(const Gate& gate)
{
    if (std::holds_alternative<BasicSingleQubitGate>(gate))
        return true;
    else if (std::holds_alternative<RZ>(gate))
        return false;
    else
    {
        const auto controlled_gate = std::get<ControlledGate>(gate);
        if (std::holds_alternative<RZ>(controlled_gate.target_gate))
            return false;
        else if (std::holds_alternative<BasicSingleQubitGate>(controlled_gate.target_gate))
            return is_clifford_plus_t(std::visit([](const auto tg){return Gate{tg};}, controlled_gate.target_gate));
        else
            LSTK_NOT_IMPLEMENTED;
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


}

namespace lsqecc {


std::string print_pi_fraction(const Fraction& fraction)
{
    return lstk::cat(fraction.num,"*pi/",fraction.den);
}

std::ostream& operator<<(std::ostream& os, const gates::BasicSingleQubitGate& gate)
{
    using namespace gates;
    
    return os << [&](){
        switch (gate.gate_type)
        {
        case BasicSingleQubitGate::Type::X: return 'x';
        case BasicSingleQubitGate::Type::Z: return 'z';
        case BasicSingleQubitGate::Type::S: return 's';
        case BasicSingleQubitGate::Type::T: return 't';
        case BasicSingleQubitGate::Type::H: return 'h';
        default:
            LSTK_UNREACHABLE;
        }
    }() << " qreg[" << gate.target_qubit << "];";
}


std::ostream& operator<<(std::ostream& os, const gates::Gate& gate)
{
    using namespace gates;

    if (const auto *basic_gate = std::get_if<BasicSingleQubitGate>(&gate))
        return os << *basic_gate;
    else if (const auto *rz_gate = std::get_if<RZ>(&gate))
    {
        LSTK_UNUSED(rz_gate);
        LSTK_NOT_IMPLEMENTED;
    }
    else
    {
        const auto controlled_gate = std::get<ControlledGate>(gate);
        if (std::holds_alternative<RZ>(controlled_gate.target_gate))
            LSTK_NOT_IMPLEMENTED;
        else if (std::holds_alternative<BasicSingleQubitGate>(controlled_gate.target_gate))
            LSTK_NOT_IMPLEMENTED;
        else
            LSTK_NOT_IMPLEMENTED;
    }
    
}


}

