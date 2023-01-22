#include <lsqecc/pipelines/slicer.hpp>

#include <lsqecc/dependency_dag/lli_dag.hpp>
#include <lsqecc/dependency_dag/gates_dag.hpp>
#include <lsqecc/gates/parse_gates.hpp>
#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
#include <lsqecc/ls_instructions/boundary_rotation_injection_stream.hpp>
#include <lsqecc/ls_instructions/teleported_s_gate_injection_stream.hpp>
#include <lsqecc/layout/ascii_layout_spec.hpp>
#include <lsqecc/layout/router.hpp>
#include <lsqecc/layout/dynamic_layouts/compact_layout.hpp>
#include <lsqecc/patches/slices_to_json.hpp>
#include <lsqecc/patches/slice.hpp>
#include <lsqecc/patches/dense_patch_computation.hpp>
#include <lsqecc/patches/slice_variant.hpp>

#include <lstk/lstk.hpp>

#include <argparse/argparse.h>
#include <nlohmann/json.hpp>

#include <cassert>
#include <iostream>
#include <string_view>
#include <sstream>
#include <stdexcept>
#include <filesystem>
#include <functional>
#include <fstream>
#include <chrono>
#include <unordered_map>


namespace lsqecc
{


namespace passes
{

// Passes have 2 types: in-place & functional
//  - In-place passess process a dag by modifying it. The dags must have the same type
//  - Functional passes a dag of one kind and output another. They need not to be the same type
// 
// Passes own their output, but hold a reference to the input


template<class OutputInstructionType>
struct DagPass
{
    using OuptutDag = DependencyDag<OutputInstructionType>;

    virtual const OuptutDag& get_dag() const = 0;
    virtual ~DagPass(){};
};


template<class OutputInstructionType>
struct DagForwardinngPass : public DagPass<OutputInstructionType>
{
    using OuptutDag = DependencyDag<OutputInstructionType>;
    DagForwardinngPass(const OuptutDag& dag_ref)
    : dag_ref_(dag_ref) {}

    const OuptutDag& get_dag() const override
    {
        return dag_ref_;
    }

private:
    const OuptutDag& dag_ref_;
};


namespace functional {


/// Give a the subdivide function to get a pass that replaces an instruction with a subdivision. E.g.:
/// --rx-- => --h--rz--h--
///
/// Used to go from Gates to Dag for example
template<class InputInstructionType, class OutputInstructionType>
struct SubdivisionDagFunctionalPass : public DagPass<OutputInstructionType>
{
    using Self = SubdivisionDagFunctionalPass<InputInstructionType, OutputInstructionType>;
    using Super = DagPass<OutputInstructionType>;
    using InputPass = DagPass<InputInstructionType>;
    using InputDag = DependencyDag<InputInstructionType>;
    using OutputDag = DependencyDag<OutputInstructionType>;
    using InputNode = Node<InputInstructionType>;
    using OutputNode = Node<OutputInstructionType>;

    SubdivisionDagFunctionalPass(std::shared_ptr<InputPass> input_pass)
    :input_pass_(input_pass), in_dag(input_pass->get_dag())
    {
        std::unordered_set<NodeIdType> visited;
        expand_subdivisions(visited, std::nullopt, std::nullopt);
    }

    virtual std::vector<OutputInstructionType> subdivide(InputInstructionType) const = 0;

    const OutputDag& get_dag() const override
    {
        return out_dag;
    }

private:
    std::shared_ptr<InputPass> input_pass_;
    const InputDag& in_dag;
    OutputDag out_dag;

    void expand_subdivisions(
        std::unordered_set<NodeIdType>& visited,
        std::optional<std::reference_wrapper<InputNode>> old_parent,
        std::optional<std::reference_wrapper<OutputNode>> new_parent
    ){
        bool is_tail = old_parent.has_value(); assert(old_parent.has_value() == new_parent.has_value());
        
        for(const std::shared_ptr<InputNode> old_child : (old_parent ? old_parent->get().future : in_dag.tails))
        {
            if (visited.contains(old_child->id)) continue;
            visited.insert(old_child->id);

            std::optional<std::reference_wrapper<OutputNode>> last_inserted = new_parent;

            auto replacement = subdivide(old_child->instruction);
            for(OutputInstructionType& new_instruction : replacement)
            {
                auto new_child = std::make_shared<OutputNode>(std::move(new_instruction), NodeCounterStruct::get_next_id());
                if(!last_inserted)
                {
                    if (is_tail) out_dag.tails.push_back(new_child);
                    else 
                    {
                        // TODO stich with the nodes that are stiched with the parent
                    }
                }
                else
                {
                    last_inserted->get().future.push_back(new_child);
                    new_child->past.push_back(*last_inserted);
                }
                last_inserted = std::ref(*new_child);
            }

            if(node_vector_contains(in_dag.heads, old_child->id))
                out_dag.heads.push_back(*last_inserted);

            expand_subdivisions(visited, std::ref(*old_child), last_inserted);
        }
    }
};


struct GatesToLLIDagPass : public SubdivisionDagFunctionalPass<gates::Gate, LSInstruction>
{
    using Super = SubdivisionDagFunctionalPass<gates::Gate, LSInstruction>;
    using Super::Super;

    std::vector<LSInstruction> subdivide(gates::Gate gate) const override
    {
        return {};
    }

    GatesToLLIDagPass(std::shared_ptr<DagPass<gates::Gate>> input_pass)
    : Super(input_pass)
    {}
};


} // namespace functional

} // namespace passes


}