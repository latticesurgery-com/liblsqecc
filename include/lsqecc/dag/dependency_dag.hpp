#pragma once

#include <lsqecc/dag/directed_graph.hpp>
#include <lsqecc/dag/commutation_trait.hpp>

#include <vector>
#include <iostream>
#include <sstream>

namespace lsqecc::dag {


/**
 * A Dag representing dependancies between instructions, where instruction can be LLI gates or any other
 * Kind of instruction that can be modelled as having dependancies.
 * 
 * Instructions must impmlement the CommutationTrait to take advantage of commutation and not be dependent
 */


template<typename Instruction>
struct DependencyDag 
{
    label_t push_instruction(Instruction&& instruction)
    {
        label_t new_instruction_label = current_label_++;
        graph_.add_node(new_instruction_label);
        instructions_[new_instruction_label] = std::move(instruction);

        for(label_t existing_instruction_label: graph_.topological_order_tails_first())
        {
            if(existing_instruction_label != new_instruction_label 
               && !CommutationTrait<Instruction>::can_commute(instructions_.at(existing_instruction_label), instructions_.at(new_instruction_label)))
                graph_.add_edge(existing_instruction_label, new_instruction_label);
        }
        return new_instruction_label;
    }

    using InstructionRef = std::reference_wrapper<const Instruction>;
    std::vector<std::pair<InstructionRef, label_t>> applicable_instructions() const
    {
        std::vector<std::pair<InstructionRef, label_t>> instructions;
        for (auto& label: graph_.heads())
            instructions.push_back({instructions_.at(label), label});

        return instructions;
    }

    Instruction take_instruction(label_t label)
    {
        Instruction instruction = std::move(instructions_.at(label));
        instructions_.erase(label);
        graph_.remove_node(label);
        return instruction;
    }


    std::ostream& to_graphviz(std::ostream& os) const
    {
        using namespace lsqecc;

        Map<label_t, std::string> nodes_contents;
        for (const auto& [label, instruction]: instructions_)
        {
            std::stringstream ss;
            ss << instruction;
            nodes_contents[label] = ss.str();
        }
        return graph_.to_graphviz(os, nodes_contents);
    }

private:
    DirectedGraph graph_;
    Map<label_t, Instruction> instructions_;
    label_t current_label_ = 0;

    template<typename T>
    friend const DirectedGraph& get_graph_for_testing(const DependencyDag<T>& g);
};


template<typename Instruction>
std::string to_graphviz(const DependencyDag<Instruction>& dag)
{
    std::stringstream ss;
    dag.to_graphviz(ss);
    return ss.str();
}


template<typename Instruction>
static const DirectedGraph& get_graph_for_testing(const DependencyDag<Instruction>& g)
{
    return g.graph_;
}

} // namespace lsqecc::dag