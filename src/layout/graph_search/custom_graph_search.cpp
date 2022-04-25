#include <lsqecc/layout/graph_search/custom_graph_search.hpp>

#include <iostream>
#include <lstk/lstk.hpp>


namespace lsqecc {


namespace custom_graph_search {


// Index in the data structure storing the list of vertices
using Vertex = size_t;

struct PredecessorData {
    std::optional<size_t> distance;
    Vertex predecessor;

    bool operator<(const PredecessorData other) const{
        return distance.value_or(std::numeric_limits<size_t>::max())
        < other.distance.value_or(std::numeric_limits<size_t>::max());
    }

    bool operator>(const PredecessorData other) const{
        return distance.value_or(std::numeric_limits<size_t>::max())
        > other.distance.value_or(std::numeric_limits<size_t>::max());
    }
};



template<bool want_cycle>
std::optional<RoutingRegion> do_graph_search_route_ancilla(
        const Slice& slice,
        PatchId source,
        PauliOperator source_op,
        PatchId target,
        PauliOperator target_op
)
{

    const Cell source_cell = slice.get_cell_by_id(source).value();
    const Cell target_cell = slice.get_cell_by_id(target).value();

    Cell furthest_cell = slice.get_layout().furthest_cell();


    auto make_vertex = [&furthest_cell](const Cell& cell) -> Vertex
    {
        return cell.row*(furthest_cell.col+1)+cell.col;
    };

    // Only used when want_cycle
    Vertex simulated_source = make_vertex(furthest_cell)+1;

    auto cell_from_vertex = [&](Vertex vertex) -> Cell
    {
        if constexpr (want_cycle)
        {
            // In this case we add a simulate source at the end of the list of vertices
            if(vertex == simulated_source)
                return target_cell;
        }

        auto v = static_cast<Cell::CoordinateType>(vertex);
        auto col = v%(furthest_cell.col+1);
        return Cell{(v-col)/(furthest_cell.col+1), col};
    };


    Vertex source_vertex = make_vertex(source_cell);
    Vertex target_vertex = make_vertex(target_cell);


    auto have_directed_edge = [&](const Cell& a, const Cell& b) -> bool{
        if(slice.is_cell_free(a) && slice.is_cell_free(b)) return true;

        if(a == cell_from_vertex(source_vertex) && b == cell_from_vertex(target_vertex))
            return slice.have_boundary_of_type_with(source_cell, b, source_op)
                && slice.have_boundary_of_type_with(target_cell, a, target_op);


        if(a == cell_from_vertex(source_vertex) && slice.is_cell_free(b))
            return slice.have_boundary_of_type_with(source_cell, b, source_op);

        if(slice.is_cell_free(a) && b == cell_from_vertex(target_vertex))
            return slice.have_boundary_of_type_with(target_cell, a, target_op);

        return false;
    };

    size_t num_vertices_on_lattice = make_vertex(furthest_cell) + 1;
    std::vector<PredecessorData> predecessor_map(num_vertices_on_lattice);
    for (size_t i = 0; i<predecessor_map.size(); ++i)
        predecessor_map[i] = PredecessorData{std::nullopt, i};


    auto cmp = [&](Vertex a, Vertex b){
        return predecessor_map[a] > predecessor_map[b]; // Note the inversion
    };
    std::priority_queue<Vertex, std::vector<Vertex>, decltype(cmp)> frontier(cmp);

    if constexpr (!want_cycle)
    {
        predecessor_map[source_vertex] = {0, source_vertex};
        frontier.push(source_vertex);
    }
    else // Simulated double source to force a cycle case
    {
        predecessor_map.push_back({0,simulated_source});

        auto neighbours = slice.get_neigbours_within_slice(source_cell);
        for(const Cell& neighbour_cell : neighbours)
        {
            if(have_directed_edge(source_cell, neighbour_cell))
            {
                Vertex neighbour = make_vertex(neighbour_cell);
                predecessor_map[neighbour] = {1, simulated_source};
                frontier.push(neighbour);
            }
        }

    }

    while(frontier.size()>0)
    {
        Vertex curr = frontier.top(); frontier.pop();
        size_t distance_to_curr = *predecessor_map[curr].distance;

        auto neighbours = slice.get_neigbours_within_slice(cell_from_vertex(curr));
        for(const Cell& neighbour_cell : neighbours)
        {
            if(!have_directed_edge(cell_from_vertex(curr), neighbour_cell))
                continue;

            Vertex neighbour = make_vertex(neighbour_cell);
            if(!predecessor_map[neighbour].distance)
            {
                predecessor_map[neighbour] = {distance_to_curr+1, curr};
                frontier.push(neighbour);
            }
            else
            {
                if(*predecessor_map[neighbour].distance > distance_to_curr+1)
                    predecessor_map[neighbour] = {distance_to_curr+1, curr};
            }
        }
    }

#if false

    std::cout<< source_vertex << " to " << target_vertex<<std::endl;

    for (size_t i = 0; i<predecessor_map.size(); ++i)
        std::cout << i << "->" << predecessor_map[i].predecessor
                  << " (" << (predecessor_map[i].distance ? std::to_string(*predecessor_map[i].distance) : "N/A") << ")" << std::endl;
#endif


    // TODO refactor this to be shared with the boost implementation
    RoutingRegion ret;

    Vertex prec = make_vertex(*slice.get_cell_by_id(target));
    Vertex curr = predecessor_map[target_vertex].predecessor;
    Vertex next = predecessor_map[curr].predecessor;

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
        next = predecessor_map[next].predecessor;
    }

    // Check if out path reached the source

    bool reached_source = curr==source_vertex;
    if constexpr (want_cycle) reached_source = curr == simulated_source;
    return reached_source ? std::make_optional(ret) : std::nullopt;
}

std::optional<RoutingRegion> graph_search_route_ancilla(
        const Slice& slice,
        PatchId source,
        PauliOperator source_op,
        PatchId target,
        PauliOperator target_op
)
{

    return source == target ?
        do_graph_search_route_ancilla<true>(slice, source, source_op, target, target_op):
        do_graph_search_route_ancilla<false>(slice, source, source_op, target, target_op);
}


}
}

