#ifndef LSQECC_GATES_HPP
#define LSQECC_GATES_HPP

#include <lsqecc/pauli_rotations/pauli_operator.hpp>

#include <cstdint>
#include <variant>


namespace lsqecc
{

using ArbitraryPrecisionInteger = int64_t;

struct Fraction{
    ArbitraryPrecisionInteger num;
    ArbitraryPrecisionInteger den;
};


using QubitNum = uint32_t;


namespace gates
{

struct BasicSingleQubitGate{
    enum class Type : uint8_t
    {
        X = static_cast<uint8_t>(PauliOperator::X),
        Z = static_cast<uint8_t>(PauliOperator::Z),
        S,
        T,
        H,

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
    ZX_WITH_MBM_TARGET_FIRST
};


static inline std::string_view CNOTType_toString(CNOTType cnot_type)
{
    using namespace std::string_view_literals;
    switch(cnot_type){
        case CNOTType::ZX_WITH_MBM_CONTROL_FIRST: return "ZXWithMBMControlFirst"sv;
        case CNOTType::ZX_WITH_MBM_TARGET_FIRST: return "ZXWithMBMTargetFirst"sv;
    }
}

static inline std::optional<CNOTType> CNOTType_fromString(std::string_view s)
{
    if(s == CNOTType_toString(CNOTType::ZX_WITH_MBM_CONTROL_FIRST)) return CNOTType::ZX_WITH_MBM_CONTROL_FIRST;
    if(s == CNOTType_toString(CNOTType::ZX_WITH_MBM_TARGET_FIRST)) return CNOTType::ZX_WITH_MBM_TARGET_FIRST;
    else return std::nullopt;
}


struct ControlledGate
{
    QubitNum control_qubit;
    SingleQubitGate target_gate;
    CNOTType cnot_type;

    static constexpr CNOTType default_cnot_type = CNOTType::ZX_WITH_MBM_CONTROL_FIRST;
};



#define MAKE_BASIC_GATE(G)\
inline constexpr BasicSingleQubitGate G(QubitNum target_qubit){\
    return BasicSingleQubitGate{target_qubit, BasicSingleQubitGate::Type::G};\
}

MAKE_BASIC_GATE(X);
MAKE_BASIC_GATE(Z);
MAKE_BASIC_GATE(S);
MAKE_BASIC_GATE(T);
MAKE_BASIC_GATE(H)

#undef MAKE_BASIC_GATE


inline constexpr ControlledGate CNOT(QubitNum target_qubit, QubitNum control_qubit, CNOTType cnot_type){
    return {target_qubit, X(target_qubit), cnot_type};
}

inline constexpr ControlledGate CRZ(QubitNum target_qubit, QubitNum control_qubit, Fraction pi_fraction, CNOTType cnot_type){
    return {target_qubit, RZ{target_qubit, pi_fraction}, cnot_type};
}

using Gate = std::variant<BasicSingleQubitGate, RZ, ControlledGate>;

} // gates namespace

std::vector<gates::Gate> to_clifford_plus_t(gates::Gate gate);

}


#endif //LSQECC_GATES_HPP
