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
    label_t push_instruction_based_on_commutation(Instruction&& instruction)
    {
        label_t new_instruction_label = add_instruction_isolated(std::move(instruction));

        for(label_t existing_instruction_label: graph_.topological_order_tails_first())
        {
            if(existing_instruction_label != new_instruction_label 
               && !CommutationTrait<Instruction>::can_commute(instructions_.at(existing_instruction_label), instructions_.at(new_instruction_label)))
                graph_.add_edge(existing_instruction_label, new_instruction_label);
        }
        return new_instruction_label;
    }

    label_t add_instruction_isolated(Instruction&& instruction)
    {
        label_t new_instruction_label = current_label_++;
        graph_.add_node(new_instruction_label);
        instructions_[new_instruction_label] = std::move(instruction);
        return new_instruction_label;
    }

    bool empty() const
    {
        return graph_.empty();
    }

    std::vector<label_t> applicable_instructions() const
    {
        std::vector<label_t> instructions;
        for (auto& label: graph_.heads())
            instructions.push_back(label);

        return instructions;
    }

    std::vector<label_t> proximate_instructions()
    {
        std::vector<label_t> proximate_instructions;
        for (auto& label: proximate_heads_)
            proximate_instructions.push_back(label);
        return proximate_instructions;
    }
    // TRL 03/29/23: Removing const label for this
    Instruction& at(label_t label)
    {
        return instructions_.at(label);
    }


    void pop_head(label_t label)
    {
        Instruction instruction = std::move(instructions_.at(label));
        instructions_.erase(label);

        for(label_t predecessor: graph_.predecessors(label))
        {
            if(proximate_dependencies_.contains({predecessor, label}))
            {
                proximate_heads_.insert(predecessor);
                proximate_dependencies_.erase({predecessor, label});
            }
        }
        graph_.remove_node(label);

        if(proximate_heads_.contains(label))
            proximate_heads_.erase(label);
    }


    /// Returns the start of the replacement
    label_t expand(label_t target, std::vector<Instruction>&& replacement, bool proximate)
    {
        if(replacement.empty())
            throw std::logic_error("Cannot expand with an empty replacement");

        // First add the new instructions to the graph
        std::vector<label_t> new_lables;
        for(auto& instruction: replacement)
        {
            label_t new_label = current_label_++;
            new_lables.push_back(new_label);
            instructions_[new_label] = std::move(instruction);
        }

        graph_.expand(target, new_lables);
        if(proximate)
            for (size_t i = 0; i < replacement.size()-1; i++)
                proximate_dependencies_.insert({new_lables[i], new_lables[i+1]});
        

        // Now do the proximity bookkeeping for the extremes of the replacement
        for (auto&[from, to]: proximate_dependencies_)
        {
            if (from == target)
            {
                proximate_dependencies_.erase({from, to});
                proximate_dependencies_.insert({new_lables.back(), to});
            }
            if (to == target)
            {
                proximate_dependencies_.erase({from, to});
                proximate_dependencies_.insert({from, new_lables.front()});
            }
        }

        if (proximate_heads_.contains(target))
        {
            proximate_heads_.erase(target);
            proximate_heads_.insert(new_lables.front());
        }

        return new_lables.front();
    }

    void make_proximate(label_t head)
    {
        proximate_heads_.insert(head);
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

        std::stringstream ss;

        if(proximate_heads_.size() > 0)
            for (auto label: proximate_heads_)
                ss << "  " << label << " [fontcolor=red];\n";

        if(proximate_dependencies_.size() > 0)
        {
            for (const auto& [from, to]: proximate_dependencies_)
                ss << "  " << from << " -> " << to << " [penwidth=5];\n";
        }


        return graph_.to_graphviz(os, nodes_contents, std::move(std::make_optional(std::move(ss))));
    }

private:
    
    // The directed graph representing the instructions and their dependencies. The graph_ only tracks the dependencies
    // while the actual instructions are stored in the instructions_ map
    DirectedGraph graph_;
    Map<label_t, Instruction> instructions_;
    
    // A set of pairs of instructions that have a proximate dependency relationship, these are instructions that must be
    // executed on subsequent slices
    SetOfPairs<label_t, label_t> proximate_dependencies_;
    
    // When an instruction that had proximate dependencies is removed, the dependant instructions are added to this set
    // to track the fact that they must be added to the next slices
    Set<label_t> proximate_heads_;
    
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