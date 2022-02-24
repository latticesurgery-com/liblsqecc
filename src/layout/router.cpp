
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
        auto route =boost_graph_search::graph_search_route_ancilla(
                slice, source, source_op, target, target_op);
        if(!route) return std::nullopt;
        cached_routes_.insert({path_identifier, *route});
    }

    return cached_routes_.at(path_identifier);
}


std::optional<RoutingRegion> CachedNaiveDijkstraRouter::do_s_gate(Slice& slice, PatchId target) const
{
    return find_routing_ancilla(
            slice,
            target,
            PauliOperator::X,
            target,
            PauliOperator::Z);
}

size_t CachedNaiveDijkstraRouter::PathIdentifier::hash::operator()(
        const CachedNaiveDijkstraRouter::PathIdentifier& x) const
{
    return 0
            + std::hash<ulong>()(static_cast<ulong>(x.source_op))
            + std::hash<ulong>()(static_cast<ulong>(x.source_cell.row))
            + std::hash<ulong>()(static_cast<ulong>(x.source_cell.col))
            + std::hash<ulong>()(static_cast<ulong>(x.target_op))
            + std::hash<ulong>()(static_cast<ulong>(x.target_cell.row))
            + std::hash<ulong>()(static_cast<ulong>(x.target_cell.col));
}


std::optional<RoutingRegion>NaiveDijkstraRouter::find_routing_ancilla(
        const Slice& slice, PatchId source, PauliOperator source_op, PatchId target, PauliOperator target_op) const
{
    return boost_graph_search::graph_search_route_ancilla(
            slice, source, source_op, target, target_op);
}

std::optional<RoutingRegion>NaiveDijkstraRouter::do_s_gate(lsqecc::Slice& slice, PatchId target) const
{
    return boost_graph_search::do_s_gate_routing(slice, target);
}


}