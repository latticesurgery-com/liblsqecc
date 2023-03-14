#pragma once

#include <algorithm>
#include <tsl/ordered_map.h>
#include <tsl/ordered_set.h>
#include <vector>
#include <ostream>
#include <lstk/lstk.hpp>


namespace lsqecc {

namespace dag {

using label_t = size_t;

template <typename K, typename V>
using Map = tsl::ordered_map<K, V>;

template <typename T>
using Set = tsl::ordered_set<T>;

template <typename S, typename T>
using SetOfPairs = tsl::ordered_set<std::pair<S, T>, lstk::pair_hash<S,T>>;


struct DirectedGraph 
{
    void add_node(label_t label);

    void add_edge(label_t from, label_t to);

    void remove_edge(label_t from, label_t to);

    void remove_node(label_t target);

    std::vector<label_t> successors(label_t label) const;

    std::vector<label_t> predecessors(label_t label) const;

    void expand(label_t target, const std::vector<label_t>& replacement);

    Set<label_t> heads() const;

    Set<label_t> tails() const;

    bool empty() const;

    std::vector<label_t> topological_order_tails_first() const;

    std::ostream& to_graphviz(std::ostream& os, const Map<label_t,std::string>& nodes_contents, std::optional<std::stringstream>&& extra_content = std::nullopt) const;

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


