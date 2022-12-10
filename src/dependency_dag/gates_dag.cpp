#include <lsqecc/dependency_dag/gates_dag.hpp>

#include <lstk/lstk.hpp>

#include <sstream>

namespace lsqecc {

template<> // TODO replace with concepts
struct CommutationTrait<gates::Gate> {
    static bool may_not_commute(const gates::Gate& lhs, const gates::Gate& rhs)
    {
        return gates::get_target_gate(lhs) == gates::get_target_gate(rhs);   
    }
};


}
