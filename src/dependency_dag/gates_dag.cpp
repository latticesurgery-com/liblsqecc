#include <lsqecc/dependency_dag/gates_dag.hpp>

#include <lstk/lstk.hpp>

#include <sstream>

namespace lsqecc {

GatesDag make_dag_from_instruction_stream(std::unique_ptr<GateStream>&& gate_stream)
{
    GatesDag gate_dag;

    while(gate_stream->has_next_gate())
        gate_dag.push(gate_stream->get_next_gate());

    return gate_dag;
}

template<> // TODO replace with concepts
struct CommutationTrait<gates::Gate> {
    static bool may_not_commute(const gates::Gate& lhs, const gates::Gate& rhs)
    {
        return gates::get_target_qubit(lhs) == gates::get_target_qubit(rhs);   
    }
};


}
