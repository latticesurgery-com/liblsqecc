#include <lsqecc/patches/voxel_slicing.hpp>

#include <iostream>

namespace lsqecc
{

using Cell3d = std::pair<size_t, Cell>;

double euclidean_distance_3d(std::pair<size_t,Cell> lhs, std::pair<size_t,Cell> rhs)
{
    return std::sqrt(
        std::pow(lhs.first - rhs.first, 2) +
        std::pow(lhs.second.row - rhs.second.row, 2) +
        std::pow(lhs.second.col - rhs.second.col, 2)
    );
}

bool has_quiescent_qubit(const DenseSlice& slice, PatchId qubit_id)
{
    if(!slice.get_patch_by_id(qubit_id))
        return false;
    return !slice.get_patch_by_id(qubit_id)->get().is_active();
}

std::optional<size_t> get_beginning_of_inactivity(
    const std::vector<DenseSlice>& volume,
    PatchId qubit_id)
{

    if(!volume.back().get_patch_by_id(qubit_id))
        throw std::runtime_error{lstk::cat("Qubit ", qubit_id, " not at the back of the volume")};

    if(!has_quiescent_qubit(volume.back(), qubit_id))
        return std::nullopt;

    for (size_t i = volume.size()-1; i >= 0 && has_quiescent_qubit(volume.at(i), qubit_id); --i)
    {
       // Check that this is the beginning either because it's the start, or because there is activity before
        if(i==0 || !has_quiescent_qubit(volume.at(i-1), qubit_id))
            return i;
    }

    throw std::runtime_error{lstk::cat("Qubit ", qubit_id, " either never free on volume before disappearing ")};
}

using VertexLabel = size_t;


struct PredecessorData {
    std::optional<size_t> distance;
    VertexLabel predecessor;

    double cost() const
    {
        return static_cast<double>(distance.value_or(std::numeric_limits<size_t>::max()));
    }
};



class VolumeSearchAdaptor
{
public:
    explicit VolumeSearchAdaptor(
            const std::vector<DenseSlice>& volume,
            size_t source_slice,
            Cell source_cell,
            PauliOperator source_op,
            size_t target_slice,
            Cell target_cell,
            PauliOperator target_op
            ) : volume_(volume),
              source_cell_(source_cell),
              target_cell_(target_cell),
              source_vertex_(make_vertex({source_slice,source_cell})),
              target_vertex_(make_vertex({target_slice,target_cell})),
              source_op_(source_op),
              target_op_(target_op)
    {}

    std::pair<size_t,Cell> furthest_cell() const
    {
        return {volume_.size(),volume_.back().get_layout().furthest_cell()};
    }

    Cell furthest_cell_on_slice() const
    {
        return volume_.back().get_layout().furthest_cell();
    }

    size_t num_vertices_on_lattice() const
    {
        return make_vertex(furthest_cell()) + 1;
    }

    VertexLabel source_vertex() const
    {
        return source_vertex_;
    }

    VertexLabel target_vertex() const
    {
        return target_vertex_;
    }

    bool have_directed_edge_on_slice(size_t slice_n, const Cell& a, const Cell& b) const
    {
        auto slice = volume_.at(slice_n);        
        if(slice.is_cell_free(a) && slice.is_cell_free(b)) return true;

        if(std::make_pair(slice_n, a) == cell3d_from_vertex(source_vertex_) && std::make_pair(slice_n, b) == cell3d_from_vertex(target_vertex_))
            return slice.have_boundary_of_type_with(source_cell_, b, source_op_)
                    && slice.have_boundary_of_type_with(target_cell_, a, target_op_);


        if(std::make_pair(slice_n, a) == cell3d_from_vertex(source_vertex_) && slice.is_cell_free(b))
            return slice.have_boundary_of_type_with(source_cell_, b, source_op_);

        if(slice.is_cell_free(a) && std::make_pair(slice_n,b) == cell3d_from_vertex(target_vertex_))
            return slice.have_boundary_of_type_with(target_cell_, a, target_op_);
        
        return false;
    };    
    

    bool have_undirected_edge_on_slice(size_t slice_n, const Cell& a, const Cell& b) const
    {
        return have_directed_edge_on_slice(slice_n, a, b) || have_directed_edge_on_slice(slice_n, b, a);
    };

    bool have_directed_edge(std::pair<size_t,Cell> a, std::pair<size_t,Cell> b) const
    {
        if(have_directed_edge_on_slice(a.first, a.second, b.second))
            return true;
        if(a.first == b.first+1 && a.second == b.second)
        {
            auto cell = a.second; // == b.second
            auto slice = volume_.at(a.first);
            auto next_slice = volume_.at(b.first);
            // Simplest case 2 free cells
            if(slice.is_cell_free(cell) && next_slice.is_cell_free(cell))
                return true;

            // Qubit preserved across multiple slices
            if(slice.patch_at(cell).has_value() && next_slice.patch_at(cell).has_value() 
                && slice.patch_at(cell)->id == next_slice.patch_at(cell)->id
                && !slice.patch_at(cell)->is_active() && !slice.patch_at(cell)->is_active())
                return true;
        }
            
        return false;
    };

    VertexLabel make_vertex(std::pair<size_t,Cell> cell3d) const
    {
        // std::cout << cell3d.first << "," << cell3d.second.row << "," << cell3d.second.col << " -> " << cell3d.first*(furthest_cell_on_slice().col+1)*(furthest_cell_on_slice().row+1) +
        // cell3d.second.row*(furthest_cell_on_slice().col+1)+cell3d.second.col << std::endl;
        
        return cell3d.first*(furthest_cell_on_slice().col+1)*(furthest_cell_on_slice().row+1) +
        cell3d.second.row*(furthest_cell_on_slice().col+1)+cell3d.second.col;
    }

    std::pair<size_t,Cell> cell3d_from_vertex(VertexLabel vertex) const
    {
        auto v = static_cast<Cell::CoordinateType>(vertex);
        auto slice_n = v/(furthest_cell_on_slice().col+1)/(furthest_cell_on_slice().row+1);
        auto slice_index = v%((furthest_cell_on_slice().col+1)*(furthest_cell_on_slice().row+1));
        auto col = slice_index%(furthest_cell_on_slice().col+1);

        // std::cout << vertex << " -> " << slice_n << "," << (slice_index)/(furthest_cell_on_slice().col+1) << "," << col << std::endl;
        return {slice_n, Cell{(slice_index)/(furthest_cell_on_slice().col+1), col}};
    };

    std::vector<std::pair<size_t,Cell>> get_neighbours(VertexLabel v) const
    {
        return get_neighbours(cell3d_from_vertex(v));
    }

    std::vector<std::pair<size_t,Cell>> get_neighbours(std::pair<size_t,Cell> cell3d, bool undirected=false) const
    {
        auto same_slice_neighbours = cell3d.second.get_neigbours_within_bounding_box_inclusive({0, 0}, furthest_cell_on_slice());
        std::vector<std::pair<size_t,Cell>> neighbours;
        for(auto n : same_slice_neighbours)
        {
            if(undirected ?
                have_undirected_edge_on_slice(cell3d.first, cell3d.second, n) : 
                have_directed_edge_on_slice(cell3d.first, cell3d.second, n)
              )
                neighbours.push_back({cell3d.first, n});
        }
        if(cell3d.first > 0 && (
                undirected ? 
                have_directed_edge(cell3d, {cell3d.first-1, cell3d.second}) :
                have_directed_edge(cell3d, {cell3d.first-1, cell3d.second}) || have_directed_edge({cell3d.first-1, cell3d.second}, cell3d))
          )
            neighbours.emplace_back(cell3d.first-1, cell3d.second);
        if(cell3d.first < volume_.size()-1 && (
                undirected ? 
                have_directed_edge(cell3d, {cell3d.first+1, cell3d.second}) :
                have_directed_edge(cell3d, {cell3d.first+1, cell3d.second}) || have_directed_edge({cell3d.first+1, cell3d.second}, cell3d))
          )
            neighbours.emplace_back(cell3d.first+1, cell3d.second);
        return neighbours;
    }

private:

    const std::vector<DenseSlice>& volume_;
    const Cell source_cell_;
    const Cell target_cell_;
    const VertexLabel source_vertex_;
    const VertexLabel target_vertex_;
    const PauliOperator source_op_;
    const PauliOperator target_op_;
};



std::ostream& operator<<(std::ostream& os, const std::pair<size_t,Cell>& p)
{
    return os << "(" << p.first << "," << p.second.row << "," <<  p.second.col << ")";
}

struct Comparator
{
    bool operator()(const VertexLabel& a, const VertexLabel& b) const
    {
        auto a_cell3d = volume_searcher.cell3d_from_vertex(a);
        auto b_cell3d = volume_searcher.cell3d_from_vertex(b);

        constexpr double GRAVITY = 0;

        return predecessor_map[a].cost() 
            + euclidean_distance_3d(a_cell3d, target_cell3d) 
            + a_cell3d.first * GRAVITY
          >
            predecessor_map[b].cost()
            + euclidean_distance_3d(b_cell3d, target_cell3d)
            + b_cell3d.first * GRAVITY;
    }

    std::pair<size_t,Cell> target_cell3d;
    const std::vector<PredecessorData>& predecessor_map;
    const VolumeSearchAdaptor& volume_searcher;
};



struct RoutingSpace
{
    std::vector<RoutingRegion> regions_by_slice_number;
};

void stitch_cells3d(
    SingleCellOccupiedByPatch& cell_to_stitch,
    const VolumeSearchAdaptor& volume_searcher,
    const std::pair<size_t,Cell>& a, 
    const std::pair<size_t,Cell>& b)
{
    // std::cout << "(" << a.first << "," << a.second.row << "," << a.second.col << ")";
    for (const std::pair<size_t, Cell>& neighbour: volume_searcher.get_neighbours(a, /* undirected */ true))
    {
        // std::cout << " - " << neighbour.first << "," << neighbour.second.row << "," << neighbour.second.col;
        if (neighbour.first != b.first) continue; // TODO add here indication for inter-slice connection
        if (b==neighbour || b==neighbour)
        {
            // std::cout << "C";
            auto boundary = cell_to_stitch.get_mut_boundary_with(neighbour.second);
            if (boundary.has_value()) {
                // std::cout << "S";
                boundary->get() = {.boundary_type=BoundaryType::Connected, .is_active=true};
            }
        }
    }
    // std::cout << std::endl;
} 


std::optional<RoutingSpace> do_volume_search(
        const std::vector<DenseSlice>& volume,
        size_t source_slice,
        Cell source_cell,
        PauliOperator source_op,
        size_t target_slice,
        Cell target_cell,
        PauliOperator target_op
)
{
    std::pair<size_t,Cell> source_cell3d{source_slice, source_cell};
    std::pair<size_t,Cell> target_cell3d{target_slice, target_cell};

    const VolumeSearchAdaptor volume_searcher(volume, source_slice, source_cell, source_op, target_slice, target_cell, target_op);

    size_t num_vertices_on_lattice = volume_searcher.num_vertices_on_lattice();
    std::vector<PredecessorData> predecessor_map(num_vertices_on_lattice);
    for (size_t i = 0; i<predecessor_map.size(); ++i)
        predecessor_map[i] = PredecessorData{std::nullopt, i};

    Comparator cmp{target_cell3d, predecessor_map, volume_searcher};
    std::priority_queue<VertexLabel, std::vector<VertexLabel>, decltype(cmp)> frontier(cmp);

    predecessor_map[volume_searcher.source_vertex()] = {std::make_optional(0), volume_searcher.source_vertex()};
    frontier.push(volume_searcher.source_vertex());
    
    while(frontier.size()>0)
    {
        VertexLabel curr = frontier.top(); frontier.pop();
        size_t distance_to_curr = *predecessor_map[curr].distance;
        
        if(curr == volume_searcher.target_vertex()) break;

        auto neighbours = volume_searcher.get_neighbours(curr);
        for(const std::pair<size_t,Cell>& neighbour_cell : neighbours)
        {
            if(!volume_searcher.have_directed_edge(volume_searcher.cell3d_from_vertex(curr), neighbour_cell))
                continue;

            VertexLabel neighbour = volume_searcher.make_vertex(neighbour_cell);
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
    RoutingSpace ret;
    ret.regions_by_slice_number = std::vector<RoutingRegion>(volume.size(), RoutingRegion{.cells={}});

    VertexLabel prec = volume_searcher.target_vertex();
    VertexLabel curr = predecessor_map[volume_searcher.target_vertex()].predecessor;
    VertexLabel next = predecessor_map[curr].predecessor;

    // Stitch the main body
    while (curr!=next)
    {
        
        std::pair<size_t,Cell> prec_cell3d = volume_searcher.cell3d_from_vertex(prec);
        std::pair<size_t,Cell> curr_cell3d = volume_searcher.cell3d_from_vertex(curr);
        std::pair<size_t,Cell> next_cell3d = volume_searcher.cell3d_from_vertex(next);

        auto& region_on_this_slice = ret.regions_by_slice_number.at(curr_cell3d.first).cells;

        region_on_this_slice.push_back(SingleCellOccupiedByPatch{
                    {.top=   {BoundaryType::None, false},
                     .bottom={BoundaryType::None, false},
                     .left=  {BoundaryType::None, false},
                     .right= {BoundaryType::None, false}},
                    curr_cell3d.second,
            });

        // In slice stitching
        if(prec_cell3d.first == curr_cell3d.first)
            stitch_cells3d(region_on_this_slice.back(), volume_searcher, curr_cell3d, prec_cell3d);       
        if(curr_cell3d.first == next_cell3d.first)
            stitch_cells3d(region_on_this_slice.back(), volume_searcher, curr_cell3d, next_cell3d);        

        // Inter slice stitching
        if(prec_cell3d.second == curr_cell3d.second)
        {
            ret.regions_by_slice_number.at(lstk::max(curr_cell3d.first, prec_cell3d.first)).cells.back().routing_connect_to_prec = true;
            ret.regions_by_slice_number.at(lstk::min(curr_cell3d.first, prec_cell3d.first)).cells.back().routing_connect_to_next = true;
        }

        prec = curr;
        curr = next;
        next = predecessor_map[next].predecessor;
    }

    // Check if out path reached the source

    bool reached_source = curr == volume_searcher.source_vertex();
    return reached_source ? std::make_optional(ret) : std::nullopt;
}


bool has_path_3d(
    const std::vector<DenseSlice>& volume,
    size_t source_slice,
    PatchId source,
    PauliOperator source_op,
    size_t target_slice,
    PatchId target,
    PauliOperator target_op)
{
    return do_volume_search(
        volume, 
        source_slice, volume.at(source_slice).get_cell_by_id(source).value(), source_op,
        target_slice, volume.at(target_slice).get_cell_by_id(target).value(), target_op
    ).has_value();
}


// TODO removed duplicate
void mark_routing_region(DenseSlice& slice, const RoutingRegion& routing_region, PatchActivity activity);


bool merge_patches_3d(
    std::vector<DenseSlice>& volume,
    size_t source_slice_n,
    PatchId source,
    PauliOperator source_op,
    size_t target_slice_n,
    PatchId target,
    PauliOperator target_op)
{
    Cell source_cell = volume.at(source_slice_n).get_cell_by_id(source).value();
    Cell target_cell = volume.at(target_slice_n).get_cell_by_id(target).value();

    std::optional<RoutingSpace> routing_space = do_volume_search(
        volume, 
        source_slice_n, source_cell, source_op,
        target_slice_n, target_cell, target_op
    );
    if(!routing_space) return false;

    for(size_t slice_number = 0; slice_number < volume.size(); ++slice_number)
    {
        auto& slice = volume.at(slice_number);
        auto& routing_region = routing_space->regions_by_slice_number.at(slice_number);
        mark_routing_region(slice, routing_region, PatchActivity::MultiPatchMeasurement);
    }
    
    if (routing_space->regions_by_slice_number.at(source_slice_n).cells.empty() && routing_space->regions_by_slice_number.at(target_slice_n).cells.empty())
    {
        assert(source_slice_n==target_slice_n);
        auto& slice = volume.at(source_slice_n);
        slice.get_boundary_between(source_cell,target_cell)->get().is_active=true;
        slice.get_boundary_between(target_cell,source_cell)->get().is_active=true;
    }
    else
    {
        auto& source_slice = volume.at(source_slice_n);
        auto sb = source_slice.get_boundary_between(source_cell, routing_space->regions_by_slice_number.at(source_slice_n).cells.back().cell);
        if(sb) sb->get().is_active = true;
        auto& target_slice = volume.at(target_slice_n);
        auto tb = target_slice.get_boundary_between(target_cell, routing_space->regions_by_slice_number.at(target_slice_n).cells.front().cell);
        if(tb) tb->get().is_active = true;
    }
    return true;

}

template<class R, class T, class F>
std::optional<R> zig_zag_iter(const std::vector<T>& v, F f) {
    size_t mid = v.size() / 2;
    size_t i = mid;
    size_t step = 1;
    bool going_up = true;

    while (i >= 0 && i < v.size()) {
        std::optional<R> ret = f(i, v[i]);
        if(ret) return ret;

        if (going_up) {
            i += step;
            going_up = false;
        } else {
            i -= step;
            going_up = true;
            step++;
        }
    }
    return std::nullopt;
}

// std::optional<std::pair<size_t,Cell>> wfind_free_ancilla_location_3d(const Layout& layout, const std::vector<DenseSlice>& volume)
// {
//     for(const Cell& possible_ancilla_location : layout.ancilla_location())
//     {0
//         auto maybe_res = zig_zag_iter<std::pair<size_t,Cell>>(volume, [&](size_t i, const DenseSlice& slice) -> std::optional<std::pair<size_t,Cell>>{
//             if(slice.is_cell_free(possible_ancilla_location))
//                 return std::make_pair(i, possible_ancilla_location);
//             return std::nullopt;
//         });
//         if(maybe_res)
//             return maybe_res;
//     }
//     return std::nullopt;
// }


enum class PreparedStateType {Magic, Y, Ancilla};

std::vector<std::pair<size_t,Cell>> find_dedicated_locations(std::vector<DenseSlice> volume, size_t slice, Cell cell, PreparedStateType state_type)
{
    std::vector<std::pair<size_t,Cell>> result;
    for (size_t i = 0; i < volume.size(); ++i)
    {

        auto state_locations_to_search = [&](){
            if (state_type == PreparedStateType::Y)
                return volume.at(i).layout.get().predistilled_y_states();
            else if (state_type == PreparedStateType::Magic)
                return get_magic_states_as_vector(volume.at(i));
            else
            {
                std::vector<Cell> all_cells;
                // return the whole lattice
                for (Cell::CoordinateType row = 0; row <= volume.at(i).layout.get().furthest_cell().row; ++row)
                    for (Cell::CoordinateType col = 0; col <= volume.at(i).layout.get().furthest_cell().col; ++col)
                        all_cells.emplace_back(row, col);
                return all_cells;
            }
        }();
        
        
        for(Cell state_location: state_locations_to_search)
            result.emplace_back(i, state_location);
    }

    std::sort(result.begin(), result.end(), [&](auto lhs, auto rhs){
        if(lhs.first != rhs.first)
            return lhs.first < rhs.first;
        return euclidean_distance_3d(lhs, {slice, cell}) < euclidean_distance_3d(rhs, {slice, cell});
    });
    return result;
}


VoxelizedInstructionResult do_voxelize_instruction_on_volume(
        std::vector<DenseSlice> volume,
        LSInstruction instruction,
        bool local_instructions,
        const Layout& layout,
        DenseSliceVisitor slice_visitor)
{
    const auto new_slice = [&]()
    {
        volume.push_back(volume.back());
        advance_slice(volume.back(), layout);
    };


    if (const auto* s = std::get_if<SinglePatchMeasurement>(&instruction.operation))
    {
        size_t free_slice = get_beginning_of_inactivity(volume, s->target).value_or([&](){
            new_slice();
            return volume.size()-1;
        }());

        // Erase all the remaing references to th emeasured out qubit
        for (size_t i = free_slice+1; i < volume.size(); ++i)
            volume.at(i).get_cell_by_id(s->target) = std::nullopt;

        return {.volume_after_application = volume};
    }
    else if (const auto* p = std::get_if<SingleQubitOp>(&instruction.operation))
    {
        size_t free_slice = get_beginning_of_inactivity(volume, s->target).value_or([&](){
            new_slice();
            return volume.size()-1;
        }());
        DenseSlice& slice = volume.at(free_slice);
        auto& free_target_patch = slice.get_patch_by_id(p->target).value().get();

        if (p->op==SingleQubitOp::Operator::S)
        {
            if (!merge_patches_3d(volume, free_slice,  p->target, PauliOperator::X, free_slice, p->target, PauliOperator::Z))
                throw std::runtime_error{lstk::cat(instruction,"; Could not do S gate routing on ", p->target)};
            
            return do_voxelize_instruction_on_volume(volume, LSInstruction{SingleQubitOp{p->target, SingleQubitOp::Operator::Z}}, local_instructions, layout, slice_visitor);
        }
        else
        {
            if (free_target_patch.is_active())
                throw std::logic_error{lstk::cat(instruction,"; Patch ", p->target, " is active")};
            
            if (p->op == SingleQubitOp::Operator::H)
            {
                free_target_patch.activity = PatchActivity::Unitary;
                free_target_patch.boundaries.instant_rotate();
            }
                
            return {.volume_after_application = volume};
        }
    }
    else if (const auto* m = std::get_if<MultiPatchMeasurement>(&instruction.operation))
    {
        if (local_instructions) LSTK_NOT_IMPLEMENTED;
        
        
        if (m->observable.size()!=2)
            throw std::logic_error(lstk::cat(instruction,"; Multi patch measurement only supports 2 patches currently. Got:\n", *m));
        auto pairs = m->observable.begin();
        const auto&[source_id, source_op] = *pairs++;
        const auto&[target_id, target_op] = *pairs;

        size_t retries = 1;
        do {
            size_t free_slice_source = get_beginning_of_inactivity(volume, source_id).value_or([&](){
                return volume.size()-1;
            }());
            size_t free_slice_target = get_beginning_of_inactivity(volume, target_id).value_or([&](){
                return volume.size()-1;
            }());
            
            if (merge_patches_3d(volume, free_slice_source, source_id, source_op, free_slice_target, target_id, target_op))
                return {.volume_after_application = volume};
            
            new_slice();
        } while (retries --> 0);
        throw std::logic_error{lstk::cat(instruction,"; Couldn't find room to route even after adding in a new silce")};
        
    }
    else if (const auto* init = std::get_if<PatchInit>(&instruction.operation))
    {
        auto location = [&]() -> std::optional<std::pair<size_t, Cell>> {
            if(init->place_next_to) LSTK_NOT_IMPLEMENTED;
            auto locations = find_dedicated_locations(volume, volume.size()-1, Cell{0,0}, PreparedStateType::Ancilla);
            // find the first empty one
            for(auto&[slice_n, cell]: locations)
            {
                DenseSlice& slice = volume.at(slice_n);
                if(!slice.patch_at(cell))
                {
                    // also check it has at least 2 free neighbours
                    bool free_x = false, free_y = false;
                    for(auto&[dx, dy]: std::array<std::pair<int,int>, 4>{{{-1,0},{1,0},{0,-1},{0,1}}})
                    {
                        if(!slice.patch_at({cell.row+dx, cell.col+dy}))
                        {
                            if(dx!=0) free_x = true;
                            else free_y = true;
                        }
                    }
                    
                    if(free_x && free_y)
                        return std::make_pair(slice_n, cell);
                }
            }
            return std::nullopt;
        }();
        if (!location) throw std::logic_error{lstk::cat(instruction,"; Could not allocate ancilla")};

        DenseSlice& slice = volume.at(location->first);

        slice.patch_at(location->second);
        slice.place_single_cell_sparse_patch(LayoutHelpers::basic_square_patch(location->second, std::nullopt, "Init"), false);
        slice.patch_at(location->second)->id = init->target;

        return {.volume_after_application = volume};
    }
    else if (const auto* bell_init = std::get_if<BellPairInit>(&instruction.operation)) LSTK_NOT_IMPLEMENTED_USE(*bell_init);
    else if (const auto* bell_cnot = std::get_if<BellBasedCNOT>(&instruction.operation)) LSTK_NOT_IMPLEMENTED_USE(*bell_cnot);
    else if (const auto* rotation = std::get_if<RotateSingleCellPatch>(&instruction.operation)) LSTK_NOT_IMPLEMENTED_USE(*rotation);
    else if (auto* mr = std::get_if<MagicStateRequest>(&instruction.operation))
    {
        const auto& d_times = layout.distillation_times();
        if (!d_times.size()) throw std::logic_error("No distillation times");

        std::vector<std::pair<size_t,Cell>> magic_states = find_dedicated_locations(volume, volume.size()-1, volume.back().get_cell_by_id(mr->near_patch).value(), PreparedStateType::Magic);
        size_t wait = 20; // TODO remove magic number
        while (magic_states.empty() && wait-->0) {
            new_slice();
            magic_states = find_dedicated_locations(volume, volume.size()-1, volume.back().get_cell_by_id(mr->near_patch).value(), PreparedStateType::Magic);
            
        } 

        if(magic_states.empty())
            throw std::runtime_error{lstk::cat(instruction,";Could not get magic state (TODO add wait)")};

        auto [slice_n, min_cell] = *magic_states.begin();

        auto& newly_bound_magic_state = volume.at(slice_n).patch_at(min_cell).value();
        newly_bound_magic_state.id = mr->target;
        newly_bound_magic_state.type = PatchType::Qubit;
        newly_bound_magic_state.activity = PatchActivity::None;
        volume.at(slice_n).magic_states.erase(min_cell);
        return {.volume_after_application = volume};
    }
    else if (auto* yr = std::get_if<YStateRequest>(&instruction.operation))
    {
        
        std::vector<std::pair<size_t,Cell>> prepared_state_locations 
            = find_dedicated_locations(volume, volume.size()-1, volume.back().get_cell_by_id(mr->near_patch).value(), PreparedStateType::Ancilla);
        for(auto [slice_n, prepared_state_location]: prepared_state_locations)
        {
            auto& slice = volume.at(slice_n);
            std::optional<DensePatch>& maybe_patch = slice.patch_at(prepared_state_location);
            if(!maybe_patch || (maybe_patch->type == PatchType::Qubit && !maybe_patch->id))
            {
                maybe_patch = DensePatch::from_sparse_patch(LayoutHelpers::basic_square_patch(prepared_state_location, yr->target, "|Y>"));
                return {.volume_after_application = volume};
            }
        }
        
        // Try again, but this time, we'll make a new slice
        new_slice();

        prepared_state_locations 
            = find_dedicated_locations(volume, volume.size()-1, volume.back().get_cell_by_id(mr->near_patch).value(), PreparedStateType::Ancilla);

        throw std::runtime_error{std::string{"Could not get Y state"}};           
    

    }
    else if (auto* busy_region = std::get_if<BusyRegion>(&instruction.operation)) LSTK_NOT_IMPLEMENTED_USE(*busy_region);
    else if (std::get_if<PatchReset>(&instruction.operation)) LSTK_NOT_IMPLEMENTED;
    LSTK_NOT_IMPLEMENTED;
}


VoxelizedInstructionResult voxelize_instruction_on_volume(
        std::vector<DenseSlice> volume,
        LSInstruction instruction,
        bool local_instructions,
        const Layout& layout,
        DenseSliceVisitor slice_visitor)
{
    size_t original_volume_size = volume.size();
    VoxelizedInstructionResult result = do_voxelize_instruction_on_volume(volume, instruction, local_instructions, layout, slice_visitor);
    result.new_slice_count = original_volume_size - volume.size();
    return result;
}


} // namespace lsqecc
