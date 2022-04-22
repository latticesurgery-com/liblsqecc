#ifndef LSQECC_ROUTER_HPP
#define LSQECC_ROUTER_HPP

#include <lsqecc/patches/sparse_slice.hpp>
#include <lsqecc/layout/searchable_slice.hpp>

#include <unordered_map>

namespace lsqecc {

enum class GraphSearchProvider
{
    Boost,
    Custom
};


struct Router {
    virtual std::optional<RoutingRegion> find_routing_ancilla(
                const SearchableSlice& slice,
                PatchId source,
                PauliOperator source_op,
                PatchId target,
                PauliOperator target_op
            ) const = 0;

    virtual void set_graph_search_provider(GraphSearchProvider graph_search_provider) = 0;

    virtual ~Router(){};
};


struct NaiveDijkstraRouter : public Router
{
    std::optional<RoutingRegion> find_routing_ancilla(
            const SearchableSlice& slice,
            PatchId source,
            PauliOperator source_op,
            PatchId target,
            PauliOperator target_op
    ) const override;

    void set_graph_search_provider(GraphSearchProvider graph_search_provider) override {
        graph_search_provider_ = graph_search_provider;
    };

private:
    GraphSearchProvider graph_search_provider_ = GraphSearchProvider::Custom;

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
            const SearchableSlice& slice,
            PatchId source,
            PauliOperator source_op,
            PatchId target,
            PauliOperator target_op
    ) const override;


    void set_graph_search_provider(GraphSearchProvider graph_search_provider) override {
        naive_router_.set_graph_search_provider(graph_search_provider);
    };

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
