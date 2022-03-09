#include <lsqecc/layout/graph_search/cxx_graph_based_graph_search.hpp>

#include <CXXGraph.hpp>

#include <iostream>

namespace lsqecc {

namespace boost_graph_search {

std::optional<RoutingRegion> graph_search_route_ancilla(
        const Slice& slice,
        PatchId source,
        PauliOperator source_op,
        PatchId target,
        PauliOperator target_op
)
{

    using Vertex = CXXGRAPH::Node<PatchId>;
    using Edge = CXXGRAPH::DirectedWeightedEdge<PatchId>;
    using Graph = CXXGRAPH::Graph<PatchId>;

    std::vector<Vertex> vertices;
    std::vector<Edge> edges;

    Cell furthest_cell = slice.layout.get().furthest_cell();

    auto make_vertex = [&furthest_cell](const Cell& cell) -> Vertex
    {
        auto vertex_str = "("+std::to_string(cell.row)+","+std::to_string(cell.col)+")";
        return Vertex(vertex_str, cell.row*(furthest_cell.col+1)+cell.col);
    };

    auto cell_from_vertex = [&furthest_cell](Vertex vertex) -> Cell
    {
        auto v = static_cast<Cell::CoordinateType>(vertex.getData());
        auto col = v%(furthest_cell.col+1);
        return Cell{(v-col)/(furthest_cell.col+1), col};
    };

    size_t edge_counter = 0;
    auto make_edge = [&](const Vertex& from, const Vertex& to) -> Edge
    {
        return {edge_counter++, from, to, 1};
    };


    // Add free nodes
    for (Cell::CoordinateType row_idx = 0; row_idx<=furthest_cell.row; ++row_idx)
    {
        for (Cell::CoordinateType col_idx = 0; col_idx<=furthest_cell.col; ++col_idx)
        {
            Cell current{row_idx, col_idx};
            vertices.push_back(make_vertex(current));
            std::optional<std::reference_wrapper<const Patch>> patch_of_node{slice.get_any_patch_on_cell(current)};

            bool node_is_free = !patch_of_node;
            if (node_is_free)
                for (const Cell& neighbour: current.get_neigbours_within_bounding_box_inclusive({0, 0}, furthest_cell))
                    if (slice.is_cell_free(neighbour))
                        edges.push_back(make_edge(make_vertex(neighbour), make_vertex(current)));

        }
    }

    // Add source
    const auto& source_patch = std::get_if<SingleCellOccupiedByPatch>(&slice.get_patch_by_id(source).cells);
    Vertex source_vertex = make_vertex(source_patch->cell);

    if (source_patch==nullptr) throw std::logic_error("Cannot route multi cell patches");
    for (Cell neighbour: source_patch->cell.get_neigbours_within_bounding_box_inclusive({0, 0}, furthest_cell))
    {
        auto boundary = source_patch->get_boundary_with(neighbour);
        if (boundary && boundary->boundary_type==boundary_for_operator(source_op))
            edges.push_back(make_edge(make_vertex(source_patch->cell), make_vertex(neighbour)));

    }

    // Add target
    const auto& target_patch = std::get_if<SingleCellOccupiedByPatch>(&slice.get_patch_by_id(target).cells);
    if (target_patch==nullptr) throw std::logic_error("Cannot route multi cell patches");

    // This means we are trying to do an S-gate/twist measurement so we artificially add a new source coinciding
    // with the existing one
    auto vmax = vertices.front().getData();
    for (const auto& item: vertices)
        vmax = std::max(item.getData(), vmax);

    Vertex target_vertex =
            target==source ? Vertex("simulated_target", (vmax+1)) : make_vertex(target_patch->cell);

    for (Cell neighbour: target_patch->cell.get_neigbours_within_bounding_box_inclusive({0, 0}, furthest_cell))
    {
        auto boundary = target_patch->get_boundary_with(neighbour);
        if (boundary && boundary->boundary_type==boundary_for_operator(target_op))
            edges.push_back(make_edge(make_vertex(neighbour), target_vertex));
    }

    using AbstractEdge = CXXGRAPH::Edge<PatchId>;
    std::list<const AbstractEdge*> edge_list_for_cxx_graph;
    for (const auto& edge: edges)
        edge_list_for_cxx_graph.emplace_back(&edge);

    Graph g(edge_list_for_cxx_graph);

    auto res = g.dial(source_vertex, 1);

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

    // Check if out path reached the source
    return std::nullopt;
}

std::optional<RoutingRegion> do_s_gate_routing(Slice& slice, PatchId target)
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
