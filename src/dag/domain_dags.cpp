#include <lsqecc/dag/domain_dags.hpp>



namespace lsqecc::dag
{



DependencyDag<LSInstruction> full_dependency_dag_from_instruction_stream(LSInstructionStream& instruction_stream)
{
    DependencyDag<LSInstruction> dag;
    while (instruction_stream.has_next_instruction())
        dag.push_instruction_based_on_commutation(instruction_stream.get_next_instruction());

    return dag;
}



DependencyDag<gates::Gate> full_dependency_dag_from_gate_stream(GateStream& gate_stream)
{
    DependencyDag<gates::Gate> dag;
    while (gate_stream.has_next_gate())
        dag.push_instruction_based_on_commutation(gate_stream.get_next_gate());

    return dag;
}

} // namespace lsqecc::dag

