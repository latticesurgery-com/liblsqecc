#ifndef LSQECC_CUSTOM_GRAPH_SEARCH_HPP
#define LSQECC_CUSTOM_GRAPH_SEARCH_HPP

#include <lsqecc/patches/slice.hpp>
#include <lsqecc/patches/patches.hpp>

namespace lsqecc {

namespace custom_graph_search {


std::optional<RoutingRegion> graph_search_route_ancilla(
        const Slice& slice,
        PatchId source,
        PauliOperator source_op,
        PatchId target,
        PauliOperator target_op
);

std::optional<RoutingRegion> cycle_routing(Slice& slice, PatchId target);

}

}

#endif //LSQECC_CUSTOM_GRAPH_SEARCH_HPP
