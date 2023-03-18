#include <gtest/gtest.h>

#include <lsqecc/dag/dependency_dag.hpp>


using namespace lsqecc::dag;

struct TestInstruction {
    std::string name;
    int interdependency_group;
};

std::ostream& operator<<(std::ostream& os, const TestInstruction& instruction)
{
    return os << instruction.name << " " << instruction.interdependency_group;
}


namespace lsqecc::dag {
template<>
struct CommutationTrait<TestInstruction> {
    static bool can_commute(const TestInstruction& lhs, const TestInstruction& rhs)
    {
        return lhs.interdependency_group != rhs.interdependency_group;
    }
};
} // namespace lsqecc::dag

TEST(dependency_dag, generate)
{
    DependencyDag<TestInstruction> dag;
    label_t instruction_A = dag.push_instruction_based_on_commutation({"A", 0});
    label_t instruction_B = dag.push_instruction_based_on_commutation({"B", 1});
    label_t instruction_C = dag.push_instruction_based_on_commutation({"C", 0});

    ASSERT_TRUE(get_graph_for_testing(dag).heads().contains(instruction_A));
    ASSERT_TRUE(get_graph_for_testing(dag).heads().contains(instruction_B));
    ASSERT_TRUE(get_graph_for_testing(dag).tails().contains(instruction_B));
    ASSERT_TRUE(get_graph_for_testing(dag).tails().contains(instruction_C));

    std::stringstream ss;
    ss << "digraph DirectedGraph {" << std::endl;
    ss << "  0 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>A 0</b></td></tr><tr><td><font color=\"darkgray\">node: 0</font></td></tr></table>>];" << std::endl;
    ss << "  1 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>B 1</b></td></tr><tr><td><font color=\"darkgray\">node: 1</font></td></tr></table>>];" << std::endl;
    ss << "  2 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>C 0</b></td></tr><tr><td><font color=\"darkgray\">node: 2</font></td></tr></table>>];" << std::endl;
    ss << "  0 -> 2;" << std::endl;
    ss << "}" << std::endl;

    ASSERT_EQ(ss.str(), to_graphviz(dag));

}


TEST(dependency_dag, generate_2)
{
    DependencyDag<TestInstruction> dag;
    dag.push_instruction_based_on_commutation({"A", 0});
    dag.push_instruction_based_on_commutation({"B", 1});
    dag.push_instruction_based_on_commutation({"C", 0});
    dag.push_instruction_based_on_commutation({"D", 1});

    std::stringstream ss;
    ss << "digraph DirectedGraph {" << std::endl;
    ss << "  0 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>A 0</b></td></tr><tr><td><font color=\"darkgray\">node: 0</font></td></tr></table>>];" << std::endl;
    ss << "  1 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>B 1</b></td></tr><tr><td><font color=\"darkgray\">node: 1</font></td></tr></table>>];" << std::endl;
    ss << "  2 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>C 0</b></td></tr><tr><td><font color=\"darkgray\">node: 2</font></td></tr></table>>];" << std::endl;
    ss << "  3 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>D 1</b></td></tr><tr><td><font color=\"darkgray\">node: 3</font></td></tr></table>>];" << std::endl;
    ss << "  0 -> 2;" << std::endl;
    ss << "  1 -> 3;" << std::endl;
    ss << "}" << std::endl;

    ASSERT_EQ(ss.str(), to_graphviz(dag));
}

TEST(dependency_dag, expand_non_proximate)
{
    DependencyDag<TestInstruction> dag;
    dag.push_instruction_based_on_commutation({"A", 0});
    dag.push_instruction_based_on_commutation({"B", 1});
    dag.push_instruction_based_on_commutation({"C", 0});
    dag.push_instruction_based_on_commutation({"D", 1});

    dag.expand(3,{{"E", 100}, {"F", 100}}, false);

    std::stringstream ss;
    ss << "digraph DirectedGraph {" << std::endl;
    ss << "  0 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>A 0</b></td></tr><tr><td><font color=\"darkgray\">node: 0</font></td></tr></table>>];" << std::endl;
    ss << "  1 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>B 1</b></td></tr><tr><td><font color=\"darkgray\">node: 1</font></td></tr></table>>];" << std::endl;
    ss << "  2 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>C 0</b></td></tr><tr><td><font color=\"darkgray\">node: 2</font></td></tr></table>>];" << std::endl;
    ss << "  4 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>E 100</b></td></tr><tr><td><font color=\"darkgray\">node: 4</font></td></tr></table>>];" << std::endl;
    ss << "  5 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>F 100</b></td></tr><tr><td><font color=\"darkgray\">node: 5</font></td></tr></table>>];" << std::endl;
    ss << "  0 -> 2;" << std::endl;
    ss << "  1 -> 4;" << std::endl;
    ss << "  4 -> 5;" << std::endl;
    ss << "}" << std::endl;

    ASSERT_EQ(ss.str(), to_graphviz(dag));
}

TEST(dependency_dag, expand_proximate)
{
    DependencyDag<TestInstruction> dag;
    dag.push_instruction_based_on_commutation({"A", 0});
    dag.push_instruction_based_on_commutation({"B", 1});
    dag.push_instruction_based_on_commutation({"C", 0});
    dag.push_instruction_based_on_commutation({"D", 1});

    dag.expand(3,{{"E", 100}, {"F", 100}}, true);

    std::stringstream ss;
    ss << "digraph DirectedGraph {" << std::endl;
    ss << "  0 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>A 0</b></td></tr><tr><td><font color=\"darkgray\">node: 0</font></td></tr></table>>];" << std::endl;
    ss << "  1 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>B 1</b></td></tr><tr><td><font color=\"darkgray\">node: 1</font></td></tr></table>>];" << std::endl;
    ss << "  2 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>C 0</b></td></tr><tr><td><font color=\"darkgray\">node: 2</font></td></tr></table>>];" << std::endl;
    ss << "  4 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>E 100</b></td></tr><tr><td><font color=\"darkgray\">node: 4</font></td></tr></table>>];" << std::endl;
    ss << "  5 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>F 100</b></td></tr><tr><td><font color=\"darkgray\">node: 5</font></td></tr></table>>];" << std::endl;
    ss << "  0 -> 2;" << std::endl;
    ss << "  1 -> 4;" << std::endl;
    ss << "  4 -> 5;" << std::endl;
    ss << "  4 -> 5 [penwidth=5];" << std::endl;
    ss << "}" << std::endl;

    ASSERT_EQ(ss.str(), to_graphviz(dag));
}


TEST(dependency_dag, proximate_heads)
{
    DependencyDag<TestInstruction> dag;
    dag.push_instruction_based_on_commutation({"A", 0});
    dag.push_instruction_based_on_commutation({"B", 1});
    dag.push_instruction_based_on_commutation({"C", 0});
    dag.push_instruction_based_on_commutation({"D", 1});

    dag.expand(3,{{"E", 100}, {"F", 100}}, true);
    dag.pop_head(5);

    std::stringstream ss;
    ss << "digraph DirectedGraph {" << std::endl;
    ss << "  0 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>A 0</b></td></tr><tr><td><font color=\"darkgray\">node: 0</font></td></tr></table>>];" << std::endl;
    ss << "  1 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>B 1</b></td></tr><tr><td><font color=\"darkgray\">node: 1</font></td></tr></table>>];" << std::endl;
    ss << "  2 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>C 0</b></td></tr><tr><td><font color=\"darkgray\">node: 2</font></td></tr></table>>];" << std::endl;
    ss << "  4 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>E 100</b></td></tr><tr><td><font color=\"darkgray\">node: 4</font></td></tr></table>>];" << std::endl;
    ss << "  0 -> 2;" << std::endl;
    ss << "  1 -> 4;" << std::endl;
    ss << "  4 [fontcolor=red];" << std::endl;
    ss << "}" << std::endl;

    ASSERT_EQ(ss.str(), to_graphviz(dag));
}

TEST(dependency_dag, simulate_head_expansion_after_retry)
{
    DependencyDag<TestInstruction> dag;
    dag.push_instruction_based_on_commutation({"A", 0});
    dag.push_instruction_based_on_commutation({"B", 1});

    label_t new_head = dag.expand(0,{{"Z", 100}}, true);
    dag.make_proximate(new_head);
    ASSERT_EQ(2, new_head);

    std::stringstream ss;
    ss << "digraph DirectedGraph {" << std::endl;
    ss << "  1 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>B 1</b></td></tr><tr><td><font color=\"darkgray\">node: 1</font></td></tr></table>>];" << std::endl;
    ss << "  2 [shape=\"plaintext\",label=<<table cellborder=\"0\"><tr><td><b>Z 100</b></td></tr><tr><td><font color=\"darkgray\">node: 2</font></td></tr></table>>];" << std::endl;
    ss << "  2 [fontcolor=red];" << std::endl;
    ss << "}" << std::endl;

    ASSERT_EQ(ss.str(), to_graphviz(dag));
}