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

struct ControlledGate
{
    QubitNum control_qubit;
    SingleQubitGate target_gate;

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


inline constexpr ControlledGate CNOT(QubitNum target_qubit, QubitNum control_qubit){
    return {target_qubit, X(target_qubit)};
}

inline constexpr ControlledGate CRZ(QubitNum target_qubit, QubitNum control_qubit, Fraction pi_fraction){
    return {target_qubit, RZ{target_qubit, pi_fraction}};
}

using Gate = std::variant<BasicSingleQubitGate, RZ, ControlledGate>;

} // gates namespace

std::vector<gates::Gate> to_clifford_plus_t(gates::Gate gate);

}


#endif //LSQECC_GATES_HPP
