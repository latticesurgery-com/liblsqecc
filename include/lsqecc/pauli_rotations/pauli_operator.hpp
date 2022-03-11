#ifndef LSQECC_PAULI_OPERATOR_HPP
#define LSQECC_PAULI_OPERATOR_HPP

#include <string_view>
#include <string>
#include <stdexcept>
#include <lstk/lstk.hpp>

namespace lsqecc {

enum class PauliOperator : uint8_t {
    I,
    X,
    Y,
    Z,
};

static inline PauliOperator PauliOperator_from_string(std::string_view s)
{
    if (s=="I") return PauliOperator::I;
    if (s=="X") return PauliOperator::X;
    if (s=="Y") return PauliOperator::Y;
    if (s=="Z") return PauliOperator::Z;
    throw std::logic_error(std::string{"Not a PauliOperator:"}+std::string{s});
}

static inline std::string PauliOperator_to_string(PauliOperator p)
{
    if (p==PauliOperator::I) return "I";
    if (p==PauliOperator::X) return "X";
    if (p==PauliOperator::Y) return "Y";
    if (p==PauliOperator::Z) return "Z";
    LSTK_UNREACHABLE;

}


}
#endif //LSQECC_PAULI_OPERATOR_HPP
