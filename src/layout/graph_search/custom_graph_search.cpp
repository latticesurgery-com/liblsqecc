#include <lsqecc/layout/graph_search/custom_graph_search.hpp>

#include <algorithm>
#include <iostream>
#include <lstk/lstk.hpp>


namespace lsqecc {


namespace custom_graph_search {



// Index in the data structure storing the list of vertices
using Vertex = size_t;

struct PredecessorData {
    size_t distance;
    Vertex predecessor;
};

// Reusable, epoch-stamped predecessor map. Replaces the old per-route O(W*H) allocate+init of a
// full-lattice std::vector<PredecessorData>. A vertex is considered "set" for the current route iff
// its stamp equals the current epoch, so resetting the whole map is just an epoch bump (O(1)) instead
// of a per-cell zeroing loop. Held in a thread_local so cost is paid once (warmup) per fixed-size layout.
struct PredecessorBuffer {
    std::vector<PredecessorData> data;
    std::vector<uint32_t> stamp;        // stamp[v]==epoch  <=>  v has been assigned this route
    std::vector<uint32_t> closed_stamp; // closed_stamp[v]==epoch  <=>  v has been finalized (popped)
    uint32_t epoch = 0;

    // Begin a new route over a lattice of n vertices. Grows the buffers only when a larger layout is
    // seen (layouts are fixed-size, so this happens once). Bumping the epoch invalidates all prior state.
    void begin_epoch(size_t n)
    {
        if (data.size() < n)
        {
            data.resize(n);
            stamp.resize(n, 0);
            closed_stamp.resize(n, 0);
        }
        ++epoch;
        if (epoch == 0) // wrapped around: force a real reset so stale 0-stamps don't read as set
        {
            std::fill(stamp.begin(), stamp.end(), 0);
            std::fill(closed_stamp.begin(), closed_stamp.end(), 0);
            epoch = 1;
        }
    }

    bool is_set(Vertex v) const { return stamp[v] == epoch; }
    bool is_closed(Vertex v) const { return closed_stamp[v] == epoch; }
    void close(Vertex v) { closed_stamp[v] = epoch; }

    // +inf (as a very large finite double, matching the previous behaviour) for unset vertices.
    double cost(Vertex v) const
    {
        return is_set(v) ? static_cast<double>(data[v].distance)
                         : static_cast<double>(std::numeric_limits<size_t>::max());
    }

    std::optional<size_t> distance(Vertex v) const
    {
        if (!is_set(v)) return std::nullopt;
        return data[v].distance;
    }

    // Unset vertices report themselves as their own predecessor, preserving the self-loop sentinel the
    // old code got from initializing predecessor_map[i] = {nullopt, i}. Path reconstruction relies on it.
    Vertex predecessor(Vertex v) const { return is_set(v) ? data[v].predecessor : v; }

    void set(Vertex v, size_t dist, Vertex pred)
    {
        data[v] = {dist, pred};
        stamp[v] = epoch;
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
            return predecessor_map.cost(a) > predecessor_map.cost(b);
        }
        else if constexpr(heuristic == Heuristic::Euclidean)
        {
            return predecessor_map.cost(a) + euclidean_distance(slice_searcher.cell_from_vertex(a), target_cell) >
                   predecessor_map.cost(b) + euclidean_distance(slice_searcher.cell_from_vertex(b), target_cell);
        }
        else
        {
            throw std::runtime_error(lstk::cat("Unknown heuristic: ", static_cast<int>(heuristic)));
        }
    }

    Cell target_cell;
    const PredecessorBuffer& predecessor_map;
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

        // If both cells are free, then they have an edge (it is assumed that this function is called only on neighbors!)
        if(slice_.is_cell_free(a) && slice_.is_cell_free(b)) return true;

        // If they are source and/or target, then the appropriate boundaries need to be matched
        if(a == cell_from_vertex(source_vertex_) && b == cell_from_vertex(target_vertex_))
            return slice_.have_boundary_of_type_with(source_cell_, b, source_op_)
                    && slice_.have_boundary_of_type_with(target_cell_, a, target_op_);


        if(a == cell_from_vertex(source_vertex_) && slice_.is_cell_free(b))
            return slice_.have_boundary_of_type_with(source_cell_, b, source_op_);

        if(slice_.is_cell_free(a) && b == cell_from_vertex(target_vertex_))
            return slice_.have_boundary_of_type_with(target_cell_, a, target_op_);

        return false;
    };

    bool have_directed_edge_EDPC(const Cell& a, const Cell& b) const
    {

        // In EDPC we assign dummy patches to cells that have been touched and mark them with PatchActivity::EDPC, and also reserve boundaries
        // Thus here we allow to route through EDPC cells but make sure that boundaries have not yet been reserved
        if ((slice_.is_cell_free_or_activity(a,{PatchActivity::EDPC}) && slice_.is_cell_free_or_activity(b, {PatchActivity::EDPC}))
            && (!slice_.is_boundary_reserved(a, b)))
         return true;

        if(a == cell_from_vertex(source_vertex_) && b == cell_from_vertex(target_vertex_))
            return slice_.have_boundary_of_type_with(source_cell_, b, source_op_)
                    && slice_.have_boundary_of_type_with(target_cell_, a, target_op_);


        if(a == cell_from_vertex(source_vertex_) && slice_.is_cell_free_or_activity(b, {PatchActivity::EDPC}))
            return slice_.have_boundary_of_type_with(source_cell_, b, source_op_);

        if(slice_.is_cell_free_or_activity(a, {PatchActivity::EDPC}) && b == cell_from_vertex(target_vertex_))
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
        PauliOperator target_op,
        bool EDPC
)
{

    const Cell source_cell = slice.get_cell_by_id(source).value();
    const Cell target_cell = slice.get_cell_by_id(target).value();

    const SliceSearchAdaptor<want_cycle> slice_searcher(slice, source_cell, target_cell, source_op, target_op);

    // Number of vertices this route addresses. In the want_cycle case an extra simulated-source vertex
    // sits at index num_vertices_on_lattice (== make_vertex(furthest)+1), so the buffer needs one more slot.
    const size_t num_vertices_on_lattice = slice_searcher.num_vertices_on_lattice();
    const size_t buffer_size = want_cycle ? num_vertices_on_lattice + 1 : num_vertices_on_lattice;

    // Reusable across routes on this thread: no per-route allocation or O(W*H) init after warmup.
    thread_local PredecessorBuffer predecessor_map;
    predecessor_map.begin_epoch(buffer_size);

    Comparator<decltype(slice_searcher), heuristic> cmp{target_cell, predecessor_map, slice_searcher};
    std::priority_queue<Vertex, std::vector<Vertex>, decltype(cmp)> frontier(cmp);

    if constexpr (!want_cycle)
    {
        predecessor_map.set(slice_searcher.source_vertex(), 0, slice_searcher.source_vertex());
        frontier.push(slice_searcher.source_vertex());
    }
    else // Simulated double source to force a cycle case
    {
        Vertex simulated_source = slice_searcher.simulated_source();
        predecessor_map.set(simulated_source, 0, simulated_source);

        source_cell.for_each_neigbour_within_bounding_box_inclusive({0, 0}, slice_searcher.furthest_cell(),
            [&](const Cell& neighbour_cell)
        {
            if((!EDPC && slice_searcher.have_directed_edge(source_cell, neighbour_cell)) ||
                (EDPC && slice_searcher.have_directed_edge_EDPC(source_cell, neighbour_cell)))
            {
                Vertex neighbour = slice_searcher.make_vertex(neighbour_cell);
                predecessor_map.set(neighbour, 1, simulated_source);
                frontier.push(neighbour);
            }
        });
    }

    while(frontier.size()>0)
    {
        Vertex curr = frontier.top(); frontier.pop();

        // Closed-set guard: skip stale/duplicate pops. This hardens against the mutable-comparator
        // heap-staleness foot-gun (A* with the consistent Euclidean heuristic is otherwise fine).
        if(predecessor_map.is_closed(curr)) continue;
        predecessor_map.close(curr);

        size_t distance_to_curr = *predecessor_map.distance(curr);

        // Early exit: once the target is finalized its distance/predecessor are settled. Previously only
        // the heuristic branch broke here; with the closed set it is also safe for Dijkstra (Heuristic::None).
        if(curr == slice_searcher.target_vertex()) break;

        const Cell curr_cell = slice_searcher.cell_from_vertex(curr);
        curr_cell.for_each_neigbour_within_bounding_box_inclusive({0, 0}, slice_searcher.furthest_cell(),
            [&](const Cell& neighbour_cell)
        {
            if((!EDPC && !slice_searcher.have_directed_edge(curr_cell, neighbour_cell)) ||
                (EDPC && !slice_searcher.have_directed_edge_EDPC(curr_cell, neighbour_cell)))
                    return;

            Vertex neighbour = slice_searcher.make_vertex(neighbour_cell);
            if(!predecessor_map.is_set(neighbour))
            {
                predecessor_map.set(neighbour, distance_to_curr+1, curr);
                frontier.push(neighbour);
            }
            else
            {
                if(*predecessor_map.distance(neighbour) > distance_to_curr+1)
                    predecessor_map.set(neighbour, distance_to_curr+1, curr);
            }
        });
    }


    // TODO refactor this to be shared with the boost implementation
    RoutingRegion ret;

    Vertex prec = slice_searcher.target_vertex();
    Vertex curr = predecessor_map.predecessor(slice_searcher.target_vertex());
    Vertex next = predecessor_map.predecessor(curr);

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

        // prec_cell and next_cell are the (adjacent) path neighbours of curr_cell, already known from the
        // search — set their boundaries directly instead of re-enumerating all neighbours to rediscover them.
        const Boundary path_boundary = EDPC
                ? Boundary{.boundary_type=BoundaryType::Reserved_Label1, .is_active=true}
                : Boundary{.boundary_type=BoundaryType::Connected,       .is_active=true};
        if(auto boundary = ret.cells.back().get_mut_boundary_with(prec_cell)) boundary->get() = path_boundary;
        if(auto boundary = ret.cells.back().get_mut_boundary_with(next_cell)) boundary->get() = path_boundary;

        prec = curr;
        curr = next;
        next = predecessor_map.predecessor(next);
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
        PauliOperator target_op,
        bool EDPC
)
{
    return source == target ?
        do_graph_search_route_ancilla<true, heuristic>(slice, source, source_op, target, target_op, EDPC):
        do_graph_search_route_ancilla<false, heuristic>(slice, source, source_op, target, target_op, EDPC);
}


std::optional<RoutingRegion> graph_search_route_ancilla(
        const Slice& slice,
        PatchId source,
        PauliOperator source_op,
        PatchId target,
        PauliOperator target_op,
        Heuristic heuristic,
        bool EDPC
)
{
    if(heuristic == Heuristic::None)
        return graph_search_route_ancilla_dispatc_heuristic<Heuristic::None>(slice, source, source_op, target, target_op, EDPC);
    else if(heuristic == Heuristic::Euclidean)
        return graph_search_route_ancilla_dispatc_heuristic<Heuristic::Euclidean>(slice, source, source_op, target, target_op, EDPC);
    else
        throw std::runtime_error(lstk::cat("Unknown heuristic: ", static_cast<int>(heuristic)));
}


}
}

