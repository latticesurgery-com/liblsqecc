#ifndef LSQECC_ROUTER_HPP
#define LSQECC_ROUTER_HPP

#include <lsqecc/patches/slice.hpp>
#include <lsqecc/layout/graph_search/boost_based_graph_search.hpp>

#include <unordered_map>

namespace lsqecc {

struct Router {
    virtual std::optional<RoutingRegion> find_routing_ancilla(
                const Slice& slice,
                PatchId source,
                PauliOperator source_op,
                PatchId target,
                PauliOperator target_op
            ) const = 0;

    virtual std::optional<RoutingRegion> do_s_gate(Slice& slice, PatchId target) const = 0;

    virtual ~Router(){};
};


struct NaiveDijkstraRouter : public Router
{
    std::optional<RoutingRegion> find_routing_ancilla(
            const Slice& slice,
            PatchId source,
            PauliOperator source_op,
            PatchId target,
            PauliOperator target_op
    ) const override;

    std::optional<RoutingRegion> do_s_gate(Slice& slice, PatchId target) const override;

};

/**
 * Assumes paths are always clear. If a new patch appears on a previous route, route will go right over it.
 * TODO: Fix that by checking that the path is clear. Will require caching occupied cells
 *
 * Also assumes that all patches are LayoutHelpers::basic_square_patch
 */
struct CachedNaiveDijkstraRouter : public Router
{

    std::optional<RoutingRegion> find_routing_ancilla(
            const Slice& slice,
            PatchId source,
            PauliOperator source_op,
            PatchId target,
            PauliOperator target_op
    ) const override;

    std::optional<RoutingRegion> do_s_gate(Slice& slice, PatchId target) const override;


    struct PathIdentifier {
        Cell source_cell;
        PauliOperator source_op;
        Cell target_cell;
        PauliOperator target_op;

        bool operator==(const PathIdentifier&) const = default;
        struct hash
        {
            size_t operator()( const PathIdentifier& x ) const;
        };
    };


private:

    NaiveDijkstraRouter naive_router_;
    mutable std::unordered_map<PathIdentifier, RoutingRegion, PathIdentifier::hash> cached_routes_;
};
}


#endif //LSQECC_ROUTER_HPP
