#pragma once

#include <algorithm>
#include <tsl/ordered_map.h>
#include <tsl/ordered_set.h>
#include <vector>
#include <ostream>

namespace lsqecc {

namespace dag {

using label_t = size_t;

template <typename K, typename V>
using Map = tsl::ordered_map<K, V>;

template <typename T>
using Set = tsl::ordered_set<T>;


struct DirectedGraph 
{
    void add_node(label_t label);

    void add_edge(label_t from, label_t to);

    void remove_edge(label_t from, label_t to);

    void remove_node(label_t target);

    void subdivide(label_t target, const std::vector<label_t>& replacement);

    Set<label_t> heads() const;

    Set<label_t> tails() const;

    std::vector<label_t> topological_order_tails_first() const;

    std::ostream& to_graphviz(std::ostream& os, const Map<label_t,std::string>& nodes_contents) const;

private:
    // Minimal definition
    Map<label_t, Set<label_t>> edges_;
    
    // Cached on the fly
    Map<label_t, Set<label_t>> back_edges_;

    void topological_order_helper(label_t label, Set<label_t>& visited, std::vector<label_t>& order) const;

    friend const Map<label_t, Set<label_t>>& get_edges_for_testing(const DirectedGraph& g);
    friend const Map<label_t, Set<label_t>>& get_back_edges_for_testing(const DirectedGraph& g);
};


const Map<label_t, Set<label_t>>& get_edges_for_testing(const DirectedGraph& g);
const Map<label_t, Set<label_t>>& get_back_edges_for_testing(const DirectedGraph& g);

} // namespace dag

} // namespace lsqecc


