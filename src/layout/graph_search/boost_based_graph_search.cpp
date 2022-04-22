#include <lsqecc/layout/graph_search/boost_based_graph_search.hpp>

#ifdef ENABLE_BOOST_GRAPH_SEARCH
# include <boost/graph/graph_traits.hpp>
# include <boost/graph/adjacency_list.hpp>
# include <boost/graph/dijkstra_shortest_paths.hpp>
# include <boost/graph/graphviz.hpp>
#endif

#include <iostream>

namespace lsqecc {

namespace boost_graph_search {


#ifdef ENABLE_BOOST_GRAPH_SEARCH

std::optional<RoutingRegion> graph_search_route_ancilla(
        const SearchableSlice& slice,
        PatchId source,
        PauliOperator source_op,
        PatchId target,
        PauliOperator target_op
)
{

    using namespace boost;


    using Vertex = adjacency_list_traits<listS, vecS, directedS>::vertex_descriptor;
    using Graph = adjacency_list<listS,
                                 vecS,
                                 directedS,
                                 property<vertex_predecessor_t, Vertex>,
                                 property<edge_weight_t, int >>;
    using Edge = std::pair<Vertex, Vertex>;

    std::vector<Vertex> vertices;
    std::vector<Edge> edges;

    Cell furthest_cell = slice.furthest_cell();

    auto make_vertex = [&furthest_cell](const Cell& cell) -> Vertex
    {
        return cell.row*(furthest_cell.col+1)+cell.col;
    };

    auto cell_from_vertex = [&furthest_cell](Vertex vertex) -> Cell
    {
        auto v = static_cast<Cell::CoordinateType>(vertex);
        auto col = v%(furthest_cell.col+1);
        return Cell{(v-col)/(furthest_cell.col+1), col};
    };


    // Add free nodes
    for (Cell::CoordinateType row_idx = 0; row_idx<=furthest_cell.row; ++row_idx)
    {
        for (Cell::CoordinateType col_idx = 0; col_idx<=furthest_cell.col; ++col_idx)
        {
            Cell current{row_idx, col_idx};
            vertices.push_back(make_vertex(current));
            if (!slice.is_cell_free(current))
                for (const Cell& neighbour: current.get_neigbours_within_bounding_box_inclusive({0, 0}, furthest_cell))
                    if (slice.is_cell_free(neighbour))
                        edges.emplace_back(make_vertex(neighbour), make_vertex(current));

        }
    }

    // Add source
    const auto source_cell = *slice.get_cell_by_id(source);
    for (Cell neighbour: slice.get_neigbours_within_slice(source_cell))
    {
        if (slice.have_boundary_of_type_with(source_cell, neighbour, source_op))
            edges.emplace_back(make_vertex(source_cell), make_vertex(neighbour));

    }

    // Add target
    const auto target_cell = *slice.get_cell_by_id(target);

    // This means we are trying to do an S-gate/twist measurement so we artificially add a new source coinciding
    // with the existing one
    Vertex target_vertex =
            target==source ? (*std::max_element(vertices.begin(), vertices.end()))+1 : make_vertex(target_cell);

    for (Cell neighbour: slice.get_neigbours_within_slice(target_cell))
    {
        if (slice.have_boundary_of_type_with(target_cell, neighbour,target_op))
            edges.emplace_back(make_vertex(neighbour), target_vertex);
    }

    Graph g{edges.begin(), edges.end(), vertices.size()};

    property_map<Graph, vertex_predecessor_t>::type p
            = get(vertex_predecessor, g);

    Vertex s = vertex(make_vertex(*slice.get_cell_by_id(source)), g);
    dijkstra_shortest_paths(g, s, predecessor_map(p));

#if false
    std::ofstream dotfile ("graph");
    write_graphviz(dotfile, g);
    std::cout<<"S:"<<make_vertex(slice.get_patch_by_id(source).get_cells()[0])<<" "
             <<"T:"<<make_vertex(slice.get_patch_by_id(target).get_cells()[0])<<std::endl;
    for(int i = 0; i<vertices.size(); i++){
        std::cout << i << " prec " << p[i] <<std::endl;
        std::cout << cell_from_vertex(i).row << ", " << cell_from_vertex(i).col
                  << "->"
                  << cell_from_vertex(p[i]).row << ", " << cell_from_vertex(p[i]).col <<std::endl;
    }
    std::cout<<std::endl;
    for(auto e : edges){
        std::cout << e.first << "-->" << e.second <<std::endl;
    }
    std::cout<< "Done" <<std::endl;
#endif

    RoutingRegion ret;

    Vertex prec = make_vertex(*slice.get_cell_by_id(target));
    Vertex curr = p[target_vertex];
    Vertex next = p[curr];
    while (curr!=next)
    {
        Cell prec_cell = cell_from_vertex(prec);
        Cell curr_cell = cell_from_vertex(curr);
        Cell next_cell = cell_from_vertex(next);

        ret.cells.push_back(SingleCellOccupiedByPatch{
                {.top=   {BoundaryType::None, false},
                 .bottom={BoundaryType::None, false},
                 .left=  {BoundaryType::None, false},
                 .right= {BoundaryType::None, false}},
                curr_cell
        });

        for (const Cell& neighbour: curr_cell.get_neigbours_within_bounding_box_inclusive({0, 0}, furthest_cell))
        {
            if (prec_cell==neighbour || next_cell==neighbour)
            {
                auto boundary = ret.cells.back().get_mut_boundary_with(neighbour);
                if (boundary) boundary->get() = {.boundary_type=BoundaryType::Connected, .is_active=true};
            }
        }

        prec = curr;
        curr = next;
        next = p[next];
    }

    // Check if out path reached the source
    return curr==s ? std::make_optional(ret) : std::nullopt;
}

#else

std::optional<RoutingRegion> graph_search_route_ancilla(
        const Slice& slice,
        PatchId source,
        PauliOperator source_op,
        PatchId target,
        PauliOperator target_op
)
{
    throw std::runtime_error("Boost graph search not available");
}

#endif

std::optional<RoutingRegion> cycle_routing(SearchableSlice& slice, PatchId target)
{
    return graph_search_route_ancilla(
            slice,
            target,
            PauliOperator::X,
            target,
            PauliOperator::Z
    );
}

}
}