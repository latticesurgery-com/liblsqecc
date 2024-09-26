#ifndef LSQECC_CUSTOM_GRAPH_SEARCH_HPP
#define LSQECC_CUSTOM_GRAPH_SEARCH_HPP

#include <lsqecc/patches/slice.hpp>
namespace lsqecc {

namespace custom_graph_search {


enum class Heuristic
{
        None, // I.e. Djikstra
        Euclidean,
};


std::optional<RoutingRegion> graph_search_route_ancilla(
        const Slice& slice,
        PatchId source,
        PauliOperator source_op,
        PatchId target,
        PauliOperator target_op,
        Heuristic heuristic,
        bool EDPC
);

}

}

#endif //LSQECC_CUSTOM_GRAPH_SEARCH_HPP
