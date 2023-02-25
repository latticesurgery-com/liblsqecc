#include <lsqecc/dag/directed_graph.hpp>

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

void DirectedGraph::subdivide(label_t target, const std::vector<label_t>& replacement)
{
    if(!edges_.count(target))
        throw std::runtime_error("Cannot subdivide non-existing node: " + std::to_string(target));

    if(replacement.size() < 2)
        throw std::runtime_error("Cannot subdivide into less than 2 nodes");

    for(const auto& label : replacement)
    {
        if(edges_.count(label))
            throw std::runtime_error("Cannot subdivide into existing node");
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


std::ostream& DirectedGraph::to_graphviz(std::ostream& os) const
{

    os << "digraph DirectedGraph {" << std::endl;

    // Print all nodes
    for(const auto& [label, neighbors] : edges_)
        os << "  " << label << ";" << std::endl;

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