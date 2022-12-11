#ifndef LSQECC_DEPENDENCY_DAG_PRINTER
#define LSQECC_DEPENDENCY_DAG_PRINTER


#include <lsqecc/dependency_dag/dependency_dag.hpp>

#include <lstk/lstk.hpp>

#include <ostream>

namespace lsqecc {


template<class InstructionType>
std::string node_style(const Node<InstructionType>& node)
{
    std::stringstream instruction_text;
    instruction_text << node.instruction;
    return lstk::cat(
        "[shape=\"plaintext\",",
         "label=<",
            "<table border=\"1\" cellborder=\"0\" cellspacing=\"1\">",
                "<tr><td><b>", lstk::str_replace(instruction_text.str(),'>',"&gt;"), "</b></td></tr>",
                "<tr><td><font color=\"darkgray\">node: ", node.id, "</font></td></tr>",
            "</table>"
        ">]"
    );
}


template<class InstructionType>
std::ostream& to_graph_viz(std::ostream& os, const DependencyDag<InstructionType>& dag)
{
    os << "digraph D {\n\n";

    dag.traverse_into_the_past([&os](const Node<InstructionType>& node){
        os << " n" << node.id << ' ' << node_style(node) << ";\n";
        
        for(const auto& past_node: node.past)
            os << " n" << node.id << " -> n"<< past_node.get().id << ";\n";
    });

    return os << "\n}" << std::endl;   
}

}

#endif
