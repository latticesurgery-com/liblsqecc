#include <lsqecc/dag/directed_graph.hpp>
#include <lstk/lstk.hpp>
#include <stdexcept>


namespace lsqecc::dag {

void DirectedGraph::add_node(label_t label)
{
    if(!edges_.count(label))
    {
        edges_.insert_or_assign(label, Set<label_t>());
        back_edges_.insert_or_assign(label, Set<label_t>());
    }
}

void DirectedGraph::add_edge(label_t from, label_t to)
{
    add_node(to);
    add_node(from);
    edges_.at(from).insert(to);
    back_edges_.at(to).insert(from);
}

void DirectedGraph::remove_edge(label_t from, label_t to)
{
    if(!edges_.count(from))
        throw std::runtime_error("Cannot remove edge from non-existing node: " + std::to_string(from));

    if(!edges_.count(to))
        throw std::runtime_error("Cannot remove edge to non-existing node: " + std::to_string(to));

    edges_.at(from).erase(std::find(edges_.at(from).begin(), edges_.at(from).end(), to));
    back_edges_.at(to).erase(std::find(back_edges_.at(to).begin(), back_edges_.at(to).end(), from));
}

void DirectedGraph::remove_node(label_t target)
{
    if(!edges_.count(target))
        throw std::runtime_error("Cannot remove non-existing node: " + std::to_string(target));

    for(const auto& to : edges_.at(target))
        remove_edge(target, to);

    for(const auto& from : back_edges_.at(target))
        remove_edge(from, target);

    edges_.erase(target);
    back_edges_.erase(target);
}


std::vector<label_t> DirectedGraph::successors(label_t label) const
{
    if(!edges_.count(label))
        throw std::runtime_error("Cannot get successors of non-existing node: " + std::to_string(label));

    std::vector<label_t> successors;
    for(const auto& to : edges_.at(label))
        successors.push_back(to);
    return successors;
}


std::vector<label_t> DirectedGraph::predecessors(label_t label) const
{
    if(!edges_.count(label))
        throw std::runtime_error("Cannot get predecessors of non-existing node: " + std::to_string(label));

    std::vector<label_t> predecessors;
    for(const auto& from : back_edges_.at(label))
        predecessors.push_back(from);
    return predecessors;
}

void DirectedGraph::expand(label_t target, const std::vector<label_t>& replacement)
{
    if(!edges_.count(target))
        throw std::runtime_error("Cannot expand non-existing node: " + std::to_string(target));

    if(replacement.size() < 2)
        throw std::runtime_error("Cannot expand into less than 2 nodes");

    for(const auto& label : replacement)
    {
        if(edges_.count(label))
            throw std::runtime_error("Cannot expand into existing node");
    }

    for(std::size_t i = 1; i != replacement.size(); ++i)
        add_edge(replacement.at(i-1), replacement.at((i)));

    Set<label_t> forward_dangling;
    for(const auto& to : edges_.at(target))
        forward_dangling.insert(to);
    for(const auto& to : forward_dangling)
        remove_edge(target, to);
    for(const auto& to : forward_dangling)
        add_edge(replacement.back(), to);

    Set<label_t> backward_dangling;
    for(const auto& from : back_edges_.at(target))
        backward_dangling.insert(from);
    for(const auto& from : backward_dangling)
        remove_edge(from, target);
    for(const auto& from : backward_dangling)
        add_edge(from, replacement.front());

    remove_node(target);
}

Set<label_t> DirectedGraph::heads() const
{
    Set<label_t> heads;
    for(const auto& [label, neighbors] : edges_)
    {
        if(back_edges_.at(label).size() == 0)
            heads.insert(label);
    }
    return heads;
}

Set<label_t> DirectedGraph::tails() const
{
    Set<label_t> tails;
    for(const auto& [label, neighbors] : edges_)
    {
        if(edges_.at(label).size() == 0)
            tails.insert(label);
    }
    return tails;
}

bool DirectedGraph::empty() const
{
    return edges_.empty();
}


void DirectedGraph::topological_order_helper(label_t current, Set<label_t>& visited, std::vector<label_t>& order) const
{
    if(visited.count(current)) return;

    visited.insert(current);

    for(const auto& neighbor : edges_.at(current))
        topological_order_helper(neighbor, visited, order);

    order.push_back(current);
}



std::vector<label_t> DirectedGraph::topological_order_tails_first() const
{
    std::vector<label_t> order;
    Set<label_t> visited;
    for(const auto& head : heads())
        topological_order_helper(head, visited, order);
    return order;
}



std::ostream& DirectedGraph::to_graphviz(std::ostream& os, const Map<label_t,std::string>& nodes_contents) const
{

    os << "digraph DirectedGraph {" << std::endl;

    // Print all nodes
    for(const auto& [label, neighbors] : edges_)
    {
        if(nodes_contents.contains(label))
            os << "  "<<label << " [shape=\"plaintext\","
               << "label=<"
               << "<table cellborder=\"0\">"
               <<   "<tr><td><b>" << lstk::str_replace(nodes_contents.at(label),'>',"&gt;") << "</b></td></tr>"
               <<   "<tr><td><font color=\"darkgray\">node: " << label << "</font></td></tr>"
               << "</table>"
            << ">];" << std::endl;
        else
            os << "  " << label << ";" << std::endl;
    }

    // Print all edges
    for(const auto& [from, neighbors] : edges_)
        for(const auto& to : neighbors)
            os << "  " << from << " -> " << to << ";" << std::endl;

    os << "}" << std::endl;
    return os;
}


const Map<label_t, Set<label_t>>& get_edges_for_testing(const DirectedGraph& g)
{
    return g.edges_;
}

const Map<label_t, Set<label_t>>& get_back_edges_for_testing(const DirectedGraph& g)
{
    return g.back_edges_;
}


} // namespace lsqecc