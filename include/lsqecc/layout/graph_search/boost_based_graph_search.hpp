#ifndef LSQECC_BOOST_BASED_GRAPH_SEARCH_HPP
#define LSQECC_BOOST_BASED_GRAPH_SEARCH_HPP


#include <lsqecc/patches/slice.hpp>

namespace lsqecc {

namespace boost_graph_search {

std::optional<RoutingRegion> graph_search_route_ancilla(
        const Slice& slice,
        PatchId source,
        PauliOperator source_op,
        PatchId target,
        PauliOperator target_op
);

}

}
#endif //LSQECC_BOOST_BASED_GRAPH_SEARCH_HPP
