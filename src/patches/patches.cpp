#include <lsqecc/patches/patches.hpp>

#include <functional>

namespace lsqecc {

BoundaryType operator!(BoundaryType bt) {
    switch (bt) {
        case BoundaryType::None:
            return BoundaryType::Connected;
        case BoundaryType::Connected:
            return BoundaryType::None;
        case BoundaryType::Rough:
            return BoundaryType::Smooth;
        case BoundaryType::Smooth:
            return BoundaryType::Rough;
        case BoundaryType::Reserved_Label1:
            return BoundaryType::Reserved_Label2;
        case BoundaryType::Reserved_Label2:
            return BoundaryType::Reserved_Label1;
    }
    LSTK_UNREACHABLE;
}

CellDirection operator!(CellDirection dir) {
    switch (dir) {
        case CellDirection::top:
            return CellDirection::bottom;
        case CellDirection::bottom:
            return CellDirection::top;
        case CellDirection::left:
            return CellDirection::right;
        case CellDirection::right:
            return CellDirection::left;
    }
    LSTK_UNREACHABLE;
}

std::ostream& operator<<(std::ostream& os, BoundaryType bt)
{
    switch (bt) {
        case BoundaryType::None:
            return os << "BoundaryType::None";
        case BoundaryType::Connected:
            return os << "BoundaryType::Connected";
        case BoundaryType::Rough:
            return os << "BoundaryType::Rough";
        case BoundaryType::Smooth:
            return os << "BoundaryType::Smooth";
        case BoundaryType::Reserved_Label1:
            return os << "BoundaryType::Reserved_Label1";
        case BoundaryType::Reserved_Label2:
            return os << "BoundaryType::Reserved_Label2";
        default:
            throw std::runtime_error("BoundaryType not supported.");
    }
}


BoundaryType boundary_for_operator(PauliOperator op){
    switch (op)
    {
    case PauliOperator::X: return BoundaryType::Rough;
    case PauliOperator::Z: return BoundaryType::Smooth;
    case PauliOperator::I: throw std::logic_error("No boundary for I operator");
    case PauliOperator::Y: throw std::logic_error("No boundary for Y operator");
    }

    LSTK_UNREACHABLE;
}

PatchId global_patch_id_counter = 0;

PatchId make_new_patch_id(){
    return global_patch_id_counter++;
}

std::vector<Cell> SparsePatch::get_cells() const
{
    if(auto single_cell_patch = std::get_if<SingleCellOccupiedByPatch>(&cells))
    {
        return {single_cell_patch->cell};
    }
    else
    {
        const auto& multi_cell_patch = std::get<MultipleCellsOccupiedByPatch>(cells);
        std::vector<Cell> out_cells{multi_cell_patch.sub_cells.size()};
        for (const auto& cell: multi_cell_patch.sub_cells)
        {
            out_cells.push_back(cell.cell);
        }
        return out_cells;
    }
}


const Cell& SparsePatch::get_a_cell() const
{
    if(auto single_cell_patch = std::get_if<SingleCellOccupiedByPatch>(&cells))
    {
        return single_cell_patch->cell;
    }
    else
    {
        const auto& multi_cell_patch = std::get<MultipleCellsOccupiedByPatch>(cells);
        return multi_cell_patch.sub_cells.front().cell;
    }
}


std::ostream& operator<<(std::ostream& os, const Cell& c)
{
    return os << "(" << c.row << "," << c.col << ")";
}

std::ostream& operator<<(std::ostream& os, const Patch& p)
{
    os << "{ type: ";
    
    switch (p.type)
    {
    case PatchType::Distillation:
        os << "Distillation";
        break;
        
    case PatchType::PreparedState:
        os << "PreparedState";
        break;
        
    case PatchType::Qubit:
        os << "Qubit";
        break;
        
    case PatchType::Routing:
        os << "Routing";
        break;
        
    case PatchType::Dead:
        os << "Dead";
        break;
    
    default:
        LSTK_UNREACHABLE;
    }
    
    os << ", activity: ";
    
    switch (p.activity)
    {
        case PatchActivity::None:
            os << "None";
            break;
            
        case PatchActivity::Measurement:
            os << "Measurement";
            break;
            
        case PatchActivity::Unitary:
            os << "Unitary";
            break;
            
        case PatchActivity::Distillation:
            os << "Distillation";
            break;
            
        case PatchActivity::Dead:
            os << "Dead";
            break;
            
        case PatchActivity::MultiPatchMeasurement:
            os << "MultiPatchMeasurement";
            break;
            
        case PatchActivity::Rotation:
            os << "Rotation";
            break;

        case PatchActivity::EDPC:
            os << "EDPC";

        case PatchActivity::Reserved:
            os << "Reserved";
        
        default:
            LSTK_UNREACHABLE;
    }
    
    if (p.id)
        os << ", id: " << p.id.value();
    
    if (p.label)
        os << ", label: " << p.label.value();
    
    os << " }";
    return os;
}

void SparsePatch::visit_individual_cells_mut(std::function<void (SingleCellOccupiedByPatch&)> f)
{
    if(SingleCellOccupiedByPatch* patch = std::get_if<SingleCellOccupiedByPatch>(&cells))
    {
        f(*patch);
    }
    else
    {
        auto& single_cells = std::get<MultipleCellsOccupiedByPatch>(cells).sub_cells;
        for(auto& c: single_cells) f(c);
    }
}

void SparsePatch::visit_individual_cells(std::function<void (const SingleCellOccupiedByPatch&)> f) const
{
    const_cast<SparsePatch*>(this)->visit_individual_cells_mut(f);
}

bool SparsePatch::is_active() const
{
    if(activity!=PatchActivity::None) return true;

    bool active_boundary=false;
    visit_individual_cells([&](auto& c){
        active_boundary = active_boundary || c.has_active_boundary();
    });

    return active_boundary;
}

std::vector<Cell> Cell::get_neigbours() const
{
    return {Cell{row-1, col},
            Cell{row+1, col},
            Cell{row, col-1},
            Cell{row, col+1}};
}

std::optional<Cell> Cell::get_directional_neighbor(const Cell& origin, const Cell& furthest_cell, CellDirection dir) const
{
    switch (dir)
    {
        case CellDirection::top:
            if (row>origin.row) return Cell{row-1, col};
            else return std::nullopt;
        case CellDirection::bottom:
            if (row<furthest_cell.row) return Cell{row+1, col};
            else return std::nullopt;
        case CellDirection::left:
            if (col>origin.col) return Cell{row, col-1};
            else return std::nullopt;
        case CellDirection::right:
            if (col<furthest_cell.col) return Cell{row, col+1};
            else return std::nullopt;
    }
    LSTK_UNREACHABLE;
}

std::vector<Cell> Cell::get_neigbours_within_bounding_box_inclusive(const Cell& origin, const Cell& furthest_cell) const
{
    std::vector<Cell> neighbours;
    if (row>origin.row)        neighbours.emplace_back(Cell{row-1, col});
    if (row<furthest_cell.row) neighbours.emplace_back(Cell{row+1, col});
    if (col>origin.col)        neighbours.emplace_back(Cell{row, col-1});
    if (col<furthest_cell.col) neighbours.emplace_back(Cell{row, col+1});
    return neighbours;
}

std::optional<Boundary> SingleCellOccupiedByPatch::get_boundary_with(const Cell& neighbour) const
{
    if(neighbour == Cell{cell.row-1, cell.col})   return top;
    if(neighbour == Cell{cell.row+1, cell.col})   return bottom;
    if(neighbour == Cell{cell.row,   cell.col-1}) return left;
    if(neighbour == Cell{cell.row,   cell.col+1}) return right;

    return std::nullopt;
}

std::optional<Boundary*> SingleCellOccupiedByPatch::get_boundary_reference_with(const Cell& neighbour)
{
    if(neighbour == Cell{cell.row-1, cell.col})   return &top;
    if(neighbour == Cell{cell.row+1, cell.col})   return &bottom;
    if(neighbour == Cell{cell.row,   cell.col-1}) return &left;
    if(neighbour == Cell{cell.row,   cell.col+1}) return &right;

    return std::nullopt;
}

bool SingleCellOccupiedByPatch::have_boundary_of_type_with(PauliOperator op, const Cell& neighbour) const
{
    std::optional<Boundary> boundary = get_boundary_with(neighbour);
    if (boundary && boundary->boundary_type==boundary_for_operator(op))
        return true;
    return false;
}


std::optional<std::reference_wrapper<Boundary>> SingleCellOccupiedByPatch::get_mut_boundary_with(const Cell& neighbour)
{
    if(neighbour == Cell{cell.row-1, cell.col})   return top;
    if(neighbour == Cell{cell.row+1, cell.col})   return bottom;
    if(neighbour == Cell{cell.row,   cell.col-1}) return left;
    if(neighbour == Cell{cell.row,   cell.col+1}) return right;

    return std::nullopt;
}
bool CellBoundaries::has_active_boundary() const
{
    return top.is_active || bottom.is_active || left.is_active || right.is_active;
}

void CellBoundaries::instant_rotate() 
{
    Boundary old_right{right};
    right = top;
    top = left;
    left = bottom;
    bottom = old_right;
}

SparsePatch DensePatch::to_sparse_patch(const Cell& c) const
{
    return SparsePatch{{type, activity, id}, SingleCellOccupiedByPatch{{boundaries}, c}};
}
DensePatch DensePatch::from_sparse_patch(const SparsePatch& sp)
{
    auto* occupied_cell = std::get_if<SingleCellOccupiedByPatch>(&sp.cells);
    if(!occupied_cell) throw std::runtime_error{"DensePatch::from_sparse_patch called with a SparsePatch that occupies multiple cells"};

    return DensePatch{static_cast<Patch>(sp),
                      {.top=occupied_cell->top,
                       .bottom=occupied_cell->bottom,
                        .left=occupied_cell->left,
                       .right=occupied_cell->right}};
}
bool DensePatch::is_active() const
{
    if(activity!=PatchActivity::None) return true;
    return boundaries.has_active_boundary();
}

}
