#include <lsqecc/dependency_dag/gates_dag.hpp>

#include <lstk/lstk.hpp>

#include <sstream>

namespace lsqecc {

template<> // TODO replace with concepts
struct CommutationTrait<gates::Gate> {
    static bool may_not_commute(const gates::Gate& lhs, const gates::Gate& rhs)
    {
        if (const auto* lhs_basic_single_qubit_gate = std::get_if<gates::BasicSingleQubitGate>(&lhs))
        {
            
        } else if (const auto* lhs_rz_gate = std::get_if<gates::RZ>(&lhs))
        {

        } else if (const auto* lhs_controlled_gate = std::get_if<gates::ControlledGate>(&lhs))
        {

        } else {
            LSTK_UNREACHABLE;
        }
        
    }
};


}
