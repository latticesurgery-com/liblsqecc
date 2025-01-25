#ifndef LSQECC_GATES_HPP
#define LSQECC_GATES_HPP

#include <lsqecc/pauli_rotations/pauli_operator.hpp>
#include <lsqecc/dag/commutation_trait.hpp>

#include <cstdint>
#include <variant>


namespace lsqecc
{

using ArbitraryPrecisionInteger = int64_t;

struct Fraction{
    ArbitraryPrecisionInteger num;
    ArbitraryPrecisionInteger den;
    bool is_negative = false;

    bool operator==(const Fraction& other) const = default;
};


using QubitNum = uint32_t;


namespace gates
{

struct BasicSingleQubitGate{
    enum class Type : uint8_t
    {
        X = static_cast<uint8_t>(PauliOperator::X),
        Y = static_cast<uint8_t>(PauliOperator::Y),
        Z = static_cast<uint8_t>(PauliOperator::Z),
        S,
        T,
        H,
        
        SDg,
        TDg,
    };

    QubitNum target_qubit;
    Type gate_type;
};


struct RZ
{
    QubitNum target_qubit;
    Fraction pi_fraction; // Phase is exp(i*pi*phase_pi_fraction)
};


using SingleQubitGate = std::variant<BasicSingleQubitGate, RZ>;

enum class CNOTType
{
    ZX_WITH_MBM_CONTROL_FIRST,
    ZX_WITH_MBM_TARGET_FIRST,
    BELL_BASED,
};

std::string_view CNOTType_toString(CNOTType cnot_type);
std::optional<CNOTType> CNOTType_fromString(std::string_view s);


enum class CNOTAncillaPlacement
{
    USE_DEDICATED_CELL,
    ANCILLA_NEXT_TO_CONTROL,
    ANCILLA_NEXT_TO_TARGET
};

std::string_view CNOTAncillaPlacement_toString(CNOTAncillaPlacement v);
std::optional<CNOTAncillaPlacement> CNOTAncillaPlacement_fromString(std::string_view s);


struct ControlledGate
{
    QubitNum control_qubit;
    SingleQubitGate target_gate;
    CNOTType cnot_type;
    CNOTAncillaPlacement cnot_ancilla_placement;

    static constexpr CNOTType default_cnot_type = CNOTType::ZX_WITH_MBM_CONTROL_FIRST;
    
    // Initialized in source file. This value is updated depending on the type of layout.
    // TODO consider using a more robust pattern if we have to support multiple layouts per lsqecc_slicer execution.
    static CNOTAncillaPlacement default_ancilla_placement;
};


struct Reset
{
    std::string register_name;
    QubitNum target_qubit;  
};



#define MAKE_BASIC_GATE(G)\
inline constexpr BasicSingleQubitGate G(QubitNum target_qubit){\
    return BasicSingleQubitGate{target_qubit, BasicSingleQubitGate::Type::G};\
}

MAKE_BASIC_GATE(X);
MAKE_BASIC_GATE(Y);
MAKE_BASIC_GATE(Z);
MAKE_BASIC_GATE(S);
MAKE_BASIC_GATE(T);
MAKE_BASIC_GATE(H);
MAKE_BASIC_GATE(SDg);
MAKE_BASIC_GATE(TDg);

#undef MAKE_BASIC_GATE


inline constexpr ControlledGate CNOT(
        QubitNum target_qubit, 
        QubitNum control_qubit,
        CNOTType cnot_type = CNOTType::ZX_WITH_MBM_CONTROL_FIRST,
        CNOTAncillaPlacement cnot_ancilla_placement = CNOTAncillaPlacement::ANCILLA_NEXT_TO_CONTROL
){
    return {control_qubit, X(target_qubit), cnot_type, cnot_ancilla_placement};
}

inline constexpr ControlledGate CZ(
        QubitNum target_qubit, 
        QubitNum control_qubit,
        CNOTType cnot_type = CNOTType::ZX_WITH_MBM_CONTROL_FIRST,
        CNOTAncillaPlacement cnot_ancilla_placement = CNOTAncillaPlacement::ANCILLA_NEXT_TO_CONTROL
){
    return {control_qubit, Z(target_qubit), cnot_type, cnot_ancilla_placement};
}

inline constexpr ControlledGate CRZ(
        QubitNum target_qubit, QubitNum control_qubit, Fraction pi_fraction, CNOTType cnot_type, CNOTAncillaPlacement cnot_ancilla_placement){
    return {control_qubit, RZ{target_qubit, pi_fraction}, cnot_type, cnot_ancilla_placement};
}

using Gate = std::variant<BasicSingleQubitGate, RZ, ControlledGate, Reset>;

std::vector<Gate> decompose(const Gate& gate, double rz_precision_log_ten_negative);
bool is_decomposed(const Gate& gate);

tsl::ordered_set<QubitNum> get_operating_qubits(const Gate& gate);


} // gates namespace


std::string print_pi_fraction(const Fraction& fraction);
std::ostream& operator<<(std::ostream& os, const gates::BasicSingleQubitGate& gate);
std::ostream& operator<<(std::ostream& os, const gates::Gate& gate);


namespace dag{
template<>
struct CommutationTrait<gates::Gate>
{
    static bool can_commute(const gates::Gate& a, const gates::Gate& b)
    {
        // Handle the case of controlled gates separately because the if they only overlap in the index they commute
        if (const auto* ctrlg_a = std::get_if<gates::ControlledGate>(&a))
            if (const auto* ctrlg_b = std::get_if<gates::ControlledGate>(&b))
                if (ctrlg_a->control_qubit == ctrlg_b->control_qubit)
                    return lstk::set_intersection(
                        std::visit(gates::get_operating_qubits, ctrlg_a->target_gate), 
                        std::visit(gates::get_operating_qubits, ctrlg_b->target_gate)
                    ).empty();

        return lstk::set_intersection(gates::get_operating_qubits(a), gates::get_operating_qubits(b)).empty();
    }
};

}


}


#endif //LSQECC_GATES_HPP
