
#include <lsqecc/layout/graph_search/boost_based_graph_search.hpp>
#include <lsqecc/layout/graph_search/custom_graph_search.hpp>
#include <lsqecc/layout/router.hpp>

namespace lsqecc {



CachedNaiveDijkstraRouter::PathIdentifier path_identifier_from_ids(
        const Slice& slice,
        PatchId source,
        PauliOperator source_op,
        PatchId target,
        PauliOperator target_op)
{
    Cell source_cell = slice.get_patch_by_id(source).get_a_cell();
    Cell target_cell = slice.get_patch_by_id(source).get_a_cell();
    return CachedNaiveDijkstraRouter::PathIdentifier{source_cell, source_op, target_cell, target_op};
}

std::optional<RoutingRegion> CachedNaiveDijkstraRouter::find_routing_ancilla(const Slice& slice, PatchId source,
        PauliOperator source_op, PatchId target, PauliOperator target_op) const
{


    auto path_identifier = path_identifier_from_ids(slice, source, source_op, target, target_op);
    if(!cached_routes_.contains(path_identifier))
    {
        auto route = naive_router_.find_routing_ancilla(
                slice, source, source_op, target, target_op);
        if(!route) return std::nullopt;
        cached_routes_.insert({path_identifier, *route});
    }

    return cached_routes_.at(path_identifier);
}



size_t CachedNaiveDijkstraRouter::PathIdentifier::hash::operator()(
        const CachedNaiveDijkstraRouter::PathIdentifier& x) const
{
    return 0
            + std::hash<uint32_t>()(static_cast<uint32_t>(x.source_op))
            + std::hash<uint32_t>()(static_cast<uint32_t>(x.source_cell.row))
            + std::hash<uint32_t>()(static_cast<uint32_t>(x.source_cell.col))
            + std::hash<uint32_t>()(static_cast<uint32_t>(x.target_op))
            + std::hash<uint32_t>()(static_cast<uint32_t>(x.target_cell.row))
            + std::hash<uint32_t>()(static_cast<uint32_t>(x.target_cell.col));
}


std::optional<RoutingRegion>NaiveDijkstraRouter::find_routing_ancilla(
        const Slice& slice, PatchId source, PauliOperator source_op, PatchId target, PauliOperator target_op) const
{

    switch(graph_search_provider_)
    {
    case GraphSearchProvider::Boost:
        return boost_graph_search::graph_search_route_ancilla(slice, source, source_op, target, target_op);
    case GraphSearchProvider::Custom:
        return custom_graph_search::graph_search_route_ancilla(slice, source, source_op, target, target_op);
    }
}



}