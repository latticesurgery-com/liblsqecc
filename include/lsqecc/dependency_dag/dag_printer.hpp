#ifndef LSQECC_DEPENDENCY_DAG_PRINTER
#define LSQECC_DEPENDENCY_DAG_PRINTER


#include <lsqecc/dependency_dag/dependency_dag.hpp>
#include <ostream>

namespace lsqecc {

template<class InstructionType>
std::ostream& to_graph_viz(std::ostream& os, const DependencyDag<InstructionType>& dag)
{
    os << "Digraph D {\n\n";

    dag.traverse_into_the_past([&os](Node<InstructionType>& node){
        for(const auto& past_node: node.past)
            os << "  " << past_node.get().instruction << " -> "<< node.instruction << ";\n";
    });

    return os << "}" << std::endl;   
}

}

#endif
