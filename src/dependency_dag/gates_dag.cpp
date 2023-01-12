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
        // Add an easy case that can be used for some interesting examples
        if(gates::is_cnot(lhs) && gates::is_cnot(rhs))
            return gates::get_target_qubit(lhs) == gates::get_target_qubit(rhs);

        // A general rule that satisfies the requirement of the trait
        return !lstk::set_intersection(gates::get_operating_qubits(lhs), gates::get_operating_qubits(rhs)).empty();
    }
};


}
