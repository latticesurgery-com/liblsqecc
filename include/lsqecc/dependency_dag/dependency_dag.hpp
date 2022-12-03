#ifndef LSQECC_DEPENDENCY_DAG
#define LSQECC_DEPENDENCY_DAG

#include <vector>
#include <memory>
#include <functional>

namespace lsqecc
{


template<class InstructionType>
struct Node 
{
    std::vector<std::shared_ptr<Node<InstructionType>>> future; // The past holds the future
    std::vector<std::reference_wrapper<Node<InstructionType>>> past; // The future only talks about it
    InstructionType instruction;

    // TODO look into using address instead of ids
    using IdType = uint32_t;
    IdType id;

    static IdType id_counter = 0;
    static IdType get_next_id()
    {
        return id_counter++;
    }

};

template<class InstructionType>
void traverse_nodes_into_the_past(
    const std::vector<std::reference_wrapper<Node<InstructionType>>>& past,
    const std::function<void(Node<InstructionType>&)>& visitor)
{
    for(const auto& node: past)
    {
        visitor(node.get());
        traverse_nodes_into_the_past(node.get().past);
    }
}


template<class InstructionType>
struct DependencyDag
{
    using NodeT = Node<InstructionType>;
    using NodeId = NodeT::IdType;

    /// Stops traversing when visitor returns false
    void traverse_into_the_past(const std::function<void(NodeT&)>& visitor)
    {
        traverse_nodes_into_the_past(heads, visitor);
    }


    void push(InstructionType&& instruction)
    {
        bool found_a_dependency = false;
        auto new_node = std::make_shared<NodeT>(
            {}, {}, std::move(instruction), NodeT::get_next_id());

        heads.push_back(std::ref(new_node));

        traverse_into_the_past([&](NodeT& node)
        {
            if(!CommutationTrait<InstructionType>::may_not_commute(node.instruction, instruction))
            {
                found_a_dependency = true;
                node.future.push_back(new_node);
                release_node_from_heads(node.id);
                new_node.past.push_back(std::ref(*node));
            }
        });

        if(!found_a_dependency)
            tails.push_back(new_node);
    }


    void release_node_from_heads(NodeId node_id)
    {
        auto target = std::find(
            heads.begin(), 
            heads.end(), 
            [&](const std::reference_wrapper<NodeT>& node){
                return node.get().id == node_id;
            })
        
        if(target != heads.end())
            heads.erase(target);
    }


    std::vector<std::reference_wrapper<Node<InstructionType>>> heads; // Most recent
    std::vector<std::shared_ptr<Node<InstructionType>>> tails; // Oldest heads
};


template<class InstructionType> // TODO replace with concepts
struct CommutationTrait {
    static bool may_not_commute(const InstructionType& lhs, const InstructionType& rhs);
};


// TODO the InstructionType is frequently repeated and will have few instantiations so
// 1. See if we can make it "global" with a class
// 2. Move the code in a c++ file and manually instantiate there


}

#endif