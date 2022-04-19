#ifndef LSQECC_BOOST_BASED_GRAPH_SEARCH_HPP
#define LSQECC_BOOST_BASED_GRAPH_SEARCH_HPP


#include <lsqecc/patches/sparse_slice.hpp>
#include <lsqecc/patches/patches.hpp>

namespace lsqecc {

namespace boost_graph_search {

std::optional<RoutingRegion> graph_search_route_ancilla(
        const SparseSlice& slice,
        PatchId source,
        PauliOperator source_op,
        PatchId target,
        PauliOperator target_op
);

std::optional<RoutingRegion> cycle_routing(SparseSlice& slice, PatchId target);

}

}
#endif //LSQECC_BOOST_BASED_GRAPH_SEARCH_HPP
