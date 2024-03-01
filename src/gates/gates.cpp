# include <lsqecc/gates/gates.hpp>
# include <lsqecc/gates/pi_over_2_to_the_n_rz_gate_approximations.hpp>
# include <lsqecc/gates/gate_approximator.hpp>

#include <array>

namespace lsqecc::gates {


std::vector<Gate> decompose_CRZ_gate(const ControlledGate &crz_gate, double rz_precision_log_ten_negative)
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
    lstk::vector_extend(res, decompose(
            RZ{crz_gate.control_qubit, Fraction{rz_gate.pi_fraction.num,
                                                rz_gate.pi_fraction.den * 2}},
            rz_precision_log_ten_negative
        )
    );
    lstk::vector_extend(res, decompose(
            RZ{rz_gate.target_qubit, Fraction{rz_gate.pi_fraction.num,
                                              rz_gate.pi_fraction.den * 2}},
            rz_precision_log_ten_negative
        )
    );
    res.emplace_back(
            CNOT(rz_gate.target_qubit, crz_gate.control_qubit, crz_gate.cnot_type, crz_gate.cnot_ancilla_placement));
    lstk::vector_extend(res, decompose(
            RZ{rz_gate.target_qubit, Fraction{-rz_gate.pi_fraction.num,
                                              rz_gate.pi_fraction.den * 2}},
            rz_precision_log_ten_negative
        )
    );
    res.emplace_back(
            CNOT(rz_gate.target_qubit, crz_gate.control_qubit, crz_gate.cnot_type, crz_gate.cnot_ancilla_placement));

    return res;
}


std::vector<Gate> decompose(const Gate &gate, double rz_precision_log_ten_negative)
{
    if (const auto *basic_gate = std::get_if<BasicSingleQubitGate>(&gate))
        return {*basic_gate};
    else if (const auto *reset = std::get_if<Reset>(&gate))
        return{*reset};
    else if (const auto *rz_gate = std::get_if<RZ>(&gate))
        return approximate_RZ_gate(*rz_gate, rz_precision_log_ten_negative);
    else
    {
        const auto controlled_gate = std::get<ControlledGate>(gate);
        if (std::holds_alternative<RZ>(controlled_gate.target_gate))
            return decompose_CRZ_gate(controlled_gate, rz_precision_log_ten_negative);
        else if (std::holds_alternative<BasicSingleQubitGate>(controlled_gate.target_gate))
            return {gate};
        else
            LSTK_NOT_IMPLEMENTED;
    }
}


bool is_decomposed(const Gate& gate)
{
    if (std::holds_alternative<BasicSingleQubitGate>(gate))
        return true;
    else if (std::holds_alternative<Reset>(gate))
        return true;
    else if (std::holds_alternative<RZ>(gate))
        return false;
    else
    {
        const auto controlled_gate = std::get<ControlledGate>(gate);
        if (std::holds_alternative<RZ>(controlled_gate.target_gate))
            return false;
        else if (std::holds_alternative<BasicSingleQubitGate>(controlled_gate.target_gate))
            return is_decomposed(std::visit([](const auto tg){return Gate{tg};}, controlled_gate.target_gate));
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
        case CNOTType::BELL_BASED:
            return "BellBased"sv;
    }
    LSTK_UNREACHABLE;
}

std::optional<CNOTType> CNOTType_fromString(std::string_view s)
{
    if (s == CNOTType_toString(CNOTType::ZX_WITH_MBM_CONTROL_FIRST)) return CNOTType::ZX_WITH_MBM_CONTROL_FIRST;
    if (s == CNOTType_toString(CNOTType::ZX_WITH_MBM_TARGET_FIRST)) return CNOTType::ZX_WITH_MBM_TARGET_FIRST;
    if (s == CNOTType_toString(CNOTType::BELL_BASED)) return CNOTType::BELL_BASED;

    else return std::nullopt;
}


std::string_view CNOTAncillaPlacement_toString(CNOTAncillaPlacement v)
{
    using namespace std::string_view_literals;
    switch (v)
    {
        case CNOTAncillaPlacement::USE_DEDICATED_CELL:
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
    if (s == CNOTAncillaPlacement_toString(CNOTAncillaPlacement::USE_DEDICATED_CELL))
        return CNOTAncillaPlacement::USE_DEDICATED_CELL;
    if (s == CNOTAncillaPlacement_toString(CNOTAncillaPlacement::ANCILLA_NEXT_TO_CONTROL))
        return CNOTAncillaPlacement::ANCILLA_NEXT_TO_CONTROL;
    if (s == CNOTAncillaPlacement_toString(CNOTAncillaPlacement::ANCILLA_NEXT_TO_TARGET))
        return CNOTAncillaPlacement::ANCILLA_NEXT_TO_TARGET;
    else return std::nullopt;
}


tsl::ordered_set<QubitNum> get_operating_qubits(const Gate& gate)
{
    tsl::ordered_set<QubitNum> res;

    std::visit(lstk::overloaded{
        [&](const BasicSingleQubitGate& g){
            res.insert(g.target_qubit);
        },
        [&](const RZ& g){
            res.insert(g.target_qubit);
        },
        [&](const ControlledGate& g){
            res.insert(g.control_qubit);

            std::visit(lstk::overloaded{
                [&](const BasicSingleQubitGate& tg){
                    res.insert(tg.target_qubit);
                },
                [&](const RZ& tg){
                    res.insert(tg.target_qubit);
                },
                [&](const Reset& reset){
                    res.insert(reset.target_qubit);
                },
                [&](const auto&){
                    LSTK_UNREACHABLE;
                }
            }, g.target_gate);
        },
        [&](const auto&){
            LSTK_UNREACHABLE;
        }
    }, gate);
    return res;
}


CNOTAncillaPlacement ControlledGate::default_ancilla_placement = CNOTAncillaPlacement::USE_DEDICATED_CELL;


} // namespace lsqecc::gates

namespace lsqecc {


std::string print_pi_fraction(const Fraction& fraction)
{
    return lstk::cat(fraction.num,"*pi/",fraction.den);
}

std::ostream& operator<<(std::ostream& os, const gates::BasicSingleQubitGate& gate)
{
    using namespace gates;
    
    return os << Gate{gate};
}


std::ostream& operator<<(std::ostream& os, const gates::Gate& gate)
{
    using namespace gates;

    std::visit(lstk::overloaded{
        [&](const BasicSingleQubitGate& gate){
            os << [&]() -> std::string {
                switch (gate.gate_type)
                {
                    case BasicSingleQubitGate::Type::X: return "x";
                    case BasicSingleQubitGate::Type::Z: return "z";
                    case BasicSingleQubitGate::Type::S: return "s";
                    case BasicSingleQubitGate::Type::T: return "t";
                    case BasicSingleQubitGate::Type::H: return "h";
                    case BasicSingleQubitGate::Type::SDg: return "sdg";
                    case BasicSingleQubitGate::Type::TDg: return "tdg";
                }
                LSTK_UNREACHABLE;
            }() << " q[" << gate.target_qubit << "];";
        },
        [&](const RZ& rz){
            os << "rz("<<rz.pi_fraction.num<<"pi/"<<rz.pi_fraction.den<<") q["<<rz.target_qubit<<"];";
        },
        [&](const ControlledGate& ctrld){
            return std::visit(lstk::overloaded{
                [&](const BasicSingleQubitGate& gate){
                    os << "c" << [&]() -> std::string {
                        switch (gate.gate_type)
                        {
                            case BasicSingleQubitGate::Type::X: return "x";
                            case BasicSingleQubitGate::Type::Z: return "z";
                            case BasicSingleQubitGate::Type::S: return "s";
                            case BasicSingleQubitGate::Type::T: return "t";
                            case BasicSingleQubitGate::Type::H: return "h";
                            case BasicSingleQubitGate::Type::SDg: return "sdg";
                            case BasicSingleQubitGate::Type::TDg: return "tdg";
                        }
                        LSTK_UNREACHABLE;
                    }() << " q[" << ctrld.control_qubit << "],q[" << gate.target_qubit << "];";
                },
                [&](const RZ& rz){
                    os << "crz(";
                    if(rz.pi_fraction.num != 1)
                        os << rz.pi_fraction.num << "*pi";
                    os <<"pi/"<<rz.pi_fraction.den << ") q[" << ctrld.control_qubit << "],q[" << rz.target_qubit << "];";
                },
            }, ctrld.target_gate);
        },
        [&](const Reset& reset){
            os << "reset" << "q[" << reset.target_qubit << "]";
        }
    }, gate);

    return os;
}


}

