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



std::optional<RoutingRegion> graph_search_route_ancilla(
        const Slice& slice,
        PatchId source,
        PauliOperator source_op,
        PatchId target,
        PauliOperator target_op
)
{

    Cell furthest_cell = slice.layout.get().furthest_cell();

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


    auto get_underlying_patch = [&](PatchId patch_id) -> const SingleCellOccupiedByPatch&
    {
        const auto* patch = std::get_if<SingleCellOccupiedByPatch>(&slice.get_patch_by_id(patch_id).cells);
        if (patch==nullptr) throw std::logic_error("Cannot route multi cell patch with id "+std::to_string(patch_id));
        return *patch;
    };

    const auto& source_patch = get_underlying_patch(source);
    Vertex source_vertex = make_vertex(source_patch.cell);
    const auto& target_patch = get_underlying_patch(target);
    Vertex target_vertex = make_vertex(target_patch.cell);


    auto have_directed_edge = [&](const Cell& a, const Cell& b) -> bool{
        if(slice.is_cell_free(a) && slice.is_cell_free(b)) return true;

        if(a == cell_from_vertex(source_vertex) && b == cell_from_vertex(target_vertex))
            return source_patch.have_boundary_of_type_with(source_op, b)
                && source_patch.have_boundary_of_type_with(target_op, a);

        if(a == cell_from_vertex(source_vertex) && slice.is_cell_free(b))
            return source_patch.have_boundary_of_type_with(source_op, b);

        if(slice.is_cell_free(a) && b == cell_from_vertex(target_vertex))
            return target_patch.have_boundary_of_type_with(target_op, a);

        return false;
    };


    size_t index_of_the_last_vertex = make_vertex(furthest_cell);
    std::vector<PredecessorData> predecessor_map(index_of_the_last_vertex+1);
    for (size_t i = 0; i<predecessor_map.size(); ++i)
        predecessor_map[i] = PredecessorData{std::nullopt, i};


    predecessor_map[source_vertex] = {0, source_vertex};

    auto cmp = [&](Vertex a, Vertex b){
        return predecessor_map[a] > predecessor_map[b]; // Note the inversion
    };
    std::priority_queue<Vertex, std::vector<Vertex>, decltype(cmp)> frontier(cmp);
    frontier.push(source_vertex);

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

    Vertex prec = make_vertex(slice.get_patch_by_id(target).get_a_cell());
    Vertex curr = predecessor_map[target_vertex].predecessor;
    Vertex next = predecessor_map[curr].predecessor;
    while (curr!=next)
    {
        Cell prec_cell = cell_from_vertex(prec);
        Cell curr_cell = cell_from_vertex(curr);
        Cell next_cell = cell_from_vertex(next);

        ret.cells.push_back(SingleCellOccupiedByPatch{
                .top=   {BoundaryType::None, false},
                .bottom={BoundaryType::None, false},
                .left=  {BoundaryType::None, false},
                .right= {BoundaryType::None, false},
                .cell=curr_cell
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
    return curr==source_vertex ? std::make_optional(ret) : std::nullopt;
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

