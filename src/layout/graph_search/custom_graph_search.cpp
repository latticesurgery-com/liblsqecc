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

    double cost() const
    {
        return static_cast<double>(distance.value_or(std::numeric_limits<size_t>::max()));
    }
};



double euclidean_distance(Cell a, Cell b)
{
    return std::sqrt(std::pow(a.row - b.row, 2) + std::pow(a.col - b.col, 2));
}

template <class SliceSearcher, Heuristic heuristic>
struct Comparator
{
    bool operator()(const Vertex& a, const Vertex& b) const
    {
        if constexpr(heuristic == Heuristic::None)
        {
            return predecessor_map[a].cost() > predecessor_map[b].cost();
        }
        else if constexpr(heuristic == Heuristic::Euclidean)
        {
            return predecessor_map[a].cost() + euclidean_distance(slice_searcher.cell_from_vertex(a), target_cell) >
                   predecessor_map[b].cost() + euclidean_distance(slice_searcher.cell_from_vertex(b), target_cell);
        }
        else
        {
            throw std::runtime_error(lstk::cat("Unknown heuristic: ", static_cast<int>(heuristic)));
        }
    }

    Cell target_cell;
    const std::vector<PredecessorData> predecessor_map;
    const SliceSearcher& slice_searcher;
};


template<bool want_cycle>
class SliceSearchAdaptor
{
public:
    explicit SliceSearchAdaptor(
            const Slice& slice,
            Cell source_cell,
            Cell target_cell,
            PauliOperator source_op,
            PauliOperator target_op)
            : slice_(slice),
              source_cell_(source_cell),
              target_cell_(target_cell),
              source_vertex_(make_vertex(source_cell)),
              target_vertex_(make_vertex(target_cell)),
              source_op_(source_op),
              target_op_(target_op)
    {
        if constexpr(want_cycle)
        {
            simulated_source_ = make_vertex(furthest_cell())+1;
        }
    };

    Cell furthest_cell() const
    {
        return slice_.get_layout().furthest_cell();
    }

    size_t num_vertices_on_lattice() const
    {
        return make_vertex(furthest_cell()) + 1;
    }

    Vertex source_vertex() const
    {
        return source_vertex_;
    }

    Vertex target_vertex() const
    {
        return target_vertex_;
    }

    template<bool _want_cycle = want_cycle>
    typename std::enable_if<_want_cycle, Vertex>::type simulated_source() const
    {
        return simulated_source_;
    }

    bool have_directed_edge(const Cell& a, const Cell& b) const
    {
        if(slice_.is_cell_free(a) && slice_.is_cell_free(b)) return true;

        if(a == cell_from_vertex(source_vertex_) && b == cell_from_vertex(target_vertex_))
            return slice_.have_boundary_of_type_with(source_cell_, b, source_op_)
                    && slice_.have_boundary_of_type_with(target_cell_, a, target_op_);


        if(a == cell_from_vertex(source_vertex_) && slice_.is_cell_free(b))
            return slice_.have_boundary_of_type_with(source_cell_, b, source_op_);

        if(slice_.is_cell_free(a) && b == cell_from_vertex(target_vertex_))
            return slice_.have_boundary_of_type_with(target_cell_, a, target_op_);

        return false;
    };

    Vertex make_vertex(const Cell& cell) const
    {
        return cell.row*(furthest_cell().col+1)+cell.col;
    }


    Cell cell_from_vertex(Vertex vertex) const
    {
        if constexpr (want_cycle)
        {
            // In this case we add a simulated source at the end of the list of vertices
            if(vertex == simulated_source_)
                return target_cell_;
        }

        auto v = static_cast<Cell::CoordinateType>(vertex);
        auto col = v%(furthest_cell().col+1);
        return Cell{(v-col)/(furthest_cell().col+1), col};
    };

    std::vector<Cell> get_neighbours(Vertex v) const
    {
        return get_neighbours(cell_from_vertex(v));
    }

    std::vector<Cell> get_neighbours(Cell cell) const
    {
        return cell.get_neigbours_within_bounding_box_inclusive({0, 0}, furthest_cell());
    }

private:

    Vertex simulated_source_ = 0;

    const Slice& slice_;
    const Cell source_cell_;
    const Cell target_cell_;
    const Vertex source_vertex_;
    const Vertex target_vertex_;
    const PauliOperator source_op_;
    const PauliOperator target_op_;
};



template<bool want_cycle, Heuristic heuristic>
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

    const SliceSearchAdaptor<want_cycle> slice_searcher(slice, source_cell, target_cell, source_op, target_op);


    size_t num_vertices_on_lattice = slice_searcher.num_vertices_on_lattice();
    std::vector<PredecessorData> predecessor_map(num_vertices_on_lattice);
    for (size_t i = 0; i<predecessor_map.size(); ++i)
        predecessor_map[i] = PredecessorData{std::nullopt, i};

    Comparator<decltype(slice_searcher), heuristic> cmp{target_cell, predecessor_map, slice_searcher};
    std::priority_queue<Vertex, std::vector<Vertex>, decltype(cmp)> frontier(cmp);

    if constexpr (!want_cycle)
    {
        predecessor_map[slice_searcher.source_vertex()] = {0, slice_searcher.source_vertex()};
        frontier.push(slice_searcher.source_vertex());
    }
    else // Simulated double source to force a cycle case
    {
        Vertex simulated_source = slice_searcher.simulated_source();
        predecessor_map.push_back(PredecessorData{0,simulated_source});

        auto neighbours = slice_searcher.get_neighbours(source_cell);
        for(const Cell& neighbour_cell : neighbours)
        {
            if(slice_searcher.have_directed_edge(source_cell, neighbour_cell))
            {
                Vertex neighbour = slice_searcher.make_vertex(neighbour_cell);
                predecessor_map[neighbour] = {1, simulated_source};
                frontier.push(neighbour);
            }
        }

    }

    while(frontier.size()>0)
    {
        Vertex curr = frontier.top(); frontier.pop();
        size_t distance_to_curr = *predecessor_map[curr].distance;

        auto neighbours = slice_searcher.get_neighbours(curr);
        for(const Cell& neighbour_cell : neighbours)
        {
            if(!slice_searcher.have_directed_edge(slice_searcher.cell_from_vertex(curr), neighbour_cell))
                continue;

            Vertex neighbour = slice_searcher.make_vertex(neighbour_cell);
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

    Vertex prec = slice_searcher.target_vertex();
    Vertex curr = predecessor_map[slice_searcher.target_vertex()].predecessor;
    Vertex next = predecessor_map[curr].predecessor;

    while (curr!=next)
    {
        Cell prec_cell = slice_searcher.cell_from_vertex(prec);
        Cell curr_cell = slice_searcher.cell_from_vertex(curr);
        Cell next_cell = slice_searcher.cell_from_vertex(next);

        ret.cells.push_back(SingleCellOccupiedByPatch{
                {.top=   {BoundaryType::None, false},
                 .bottom={BoundaryType::None, false},
                 .left=  {BoundaryType::None, false},
                 .right= {BoundaryType::None, false}},
                curr_cell
        });

        for (const Cell& neighbour: slice_searcher.get_neighbours(curr_cell))
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

    bool reached_source = curr== slice_searcher.source_vertex();
    if constexpr (want_cycle) reached_source = curr == slice_searcher.simulated_source();
    return reached_source ? std::make_optional(ret) : std::nullopt;
}


template<Heuristic heuristic>
std::optional<RoutingRegion> graph_search_route_ancilla_dispatc_heuristic(
        const Slice& slice,
        PatchId source,
        PauliOperator source_op,
        PatchId target,
        PauliOperator target_op
)
{
    return source == target ?
        do_graph_search_route_ancilla<true, heuristic>(slice, source, source_op, target, target_op):
        do_graph_search_route_ancilla<false, heuristic>(slice, source, source_op, target, target_op);
}


std::optional<RoutingRegion> graph_search_route_ancilla(
        const Slice& slice,
        PatchId source,
        PauliOperator source_op,
        PatchId target,
        PauliOperator target_op,
        Heuristic heuristic
)
{
    if(heuristic == Heuristic::None)
        return graph_search_route_ancilla_dispatc_heuristic<Heuristic::None>(slice, source, source_op, target, target_op);
    else if(heuristic == Heuristic::Euclidean)
        return graph_search_route_ancilla_dispatc_heuristic<Heuristic::Euclidean>(slice, source, source_op, target, target_op);
    else
        throw std::runtime_error(lstk::cat("Unknown heuristic: ", static_cast<int>(heuristic)));
}


}
}

