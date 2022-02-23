//
// Created by george on 2022-02-15.
//

#include <lsqecc/layout/ascii_layout_spec.hpp>
#include <absl/strings/str_split.h>
#include <cppitertools/itertools.hpp>

#include <iterator>
#include <algorithm>
#include <numeric>
#include <deque>

static_assert(std::is_same_v<absl::string_view,std::string_view>);

namespace lsqecc{
std::vector<std::vector<AsciiLayoutSpec::CellType>> AsciiLayoutSpec::parse_grid(const std::string_view input)
{
    std::vector<std::vector<AsciiLayoutSpec::CellType>> rows;
    for(const std::string_view& row: absl::StrSplit(input,'\n'))
    {
        rows.emplace_back();
        for(char c: row)
        {
            rows.back().push_back(cell_type_from_char(c));
        }
    }
    return rows;
}

std::vector<Cell> AsciiLayoutSpec::connected_component(Cell start) const
{
    AsciiLayoutSpec::CellType target_type = grid_spec_[start.row][start.col];
    std::vector<Cell> connected_cells;
    std::deque<Cell> fronteer;
    fronteer.push_back(start);

    while(fronteer.size()>0)
    {
        Cell curr = fronteer.front(); fronteer.pop_front();
        connected_cells.push_back(curr);
        auto neighbours = curr.get_neigbours_within_bounding_box_inclusive({0,0}, furthest_cell());
        for (const auto &item : neighbours)
        {
            if(grid_spec_[item.row][item.col] == target_type
            && std::find(fronteer.begin(), fronteer.end(), item) == fronteer.end()
            && std::find(connected_cells.begin(), connected_cells.end(), item) == connected_cells.end())
            {
                fronteer.push_back(item);
            }
        }
    }

    return connected_cells;
}

Cell AsciiLayoutSpec::furthest_cell() const
{
    if(grid_spec_.size()==0) throw std::logic_error("Grid spec with no rows");

    size_t n_cols = grid_spec_.front().size();
    for (const auto &row : grid_spec_)
        if(n_cols != row.size())
            throw std::logic_error("All rows must have the same number of cols");

    if(n_cols==0) throw std::logic_error("Grid spec with no cols");
    return Cell::from_ints(grid_spec_.size()-1, n_cols-1);
}


MultipleCellsOccupiedByPatch make_distillation_region_starting_from(const Cell& cell, const AsciiLayoutSpec& spec)
{
    MultipleCellsOccupiedByPatch region;
    auto single_cells = iter::imap(LayoutHelpers::make_distillation_region_cell, spec.connected_component(cell));
    std::move(single_cells.begin(), single_cells.end(), std::back_inserter(region.sub_cells));
    return region;
}

std::optional<Cell> AsciiLayoutSpec::find_a_cell_of_type(AsciiLayoutSpec::CellType target) const
{
    for(const auto&[row, row_data]: iter::enumerate(grid_spec_))
        for(const auto&[col, cell]: iter::enumerate(row_data))
            if(cell == target)
                return Cell::from_ints(row,col);

    return std::nullopt;
}

std::vector<Cell> AsciiLayoutSpec::find_all_cells_of_type(AsciiLayoutSpec::CellType target) const
{
    std::vector<Cell> cells;

    for(const auto&[row, row_data]: iter::enumerate(grid_spec_))
        for(const auto&[col, cell]: iter::enumerate(row_data))
            if(cell == target)
                cells.push_back(Cell::from_ints(row,col));

    return cells;
}

std::ostream& AsciiLayoutSpec::operator<<(std::ostream& os) {
    for (const auto &row : grid_spec_)
    {
        for (const auto &item : row)
            os << item;
        os << std::endl;
    }
    return os;
}



bool is_already_in_this_distillation_region(const MultipleCellsOccupiedByPatch&  distillation_region, Cell cell)
{
    for(const auto& existing_distillation_cell : distillation_region.sub_cells)
        if(cell == existing_distillation_cell.cell)
            return true;
    return false;
}

bool is_already_in_some_distillation_region(const std::vector<MultipleCellsOccupiedByPatch>&  cached_distillation_regions, Cell cell)
{
    for(const auto& existing_distillation_region: cached_distillation_regions)
        if (is_already_in_this_distillation_region(existing_distillation_region,cell))
            return true;

    return false;
}


void LayoutFromSpec::init_cache(const AsciiLayoutSpec& spec)
{
    for(const AsciiLayoutSpec::CellType distillation_region_char : AsciiLayoutSpec::k_distillation_region_types)
    {

        auto cells_of_type = spec.find_all_cells_of_type(distillation_region_char);

        for (const auto& starting_cell: cells_of_type)
        {

            if(is_already_in_some_distillation_region(cached_distillation_regions_, starting_cell))
                continue;

            auto new_distillation_region = make_distillation_region_starting_from(starting_cell, spec);

            for(auto& cell_occupied_by_patch : new_distillation_region.sub_cells)
                for(const auto& neighbour : cell_occupied_by_patch.cell.get_neigbours())
                    cell_occupied_by_patch.get_mut_boundary_with(neighbour)->get().boundary_type =
                            is_already_in_this_distillation_region(new_distillation_region, neighbour)
                            ? BoundaryType::None : BoundaryType::Smooth;

            cached_distillation_regions_.push_back(new_distillation_region);
            cached_distillation_times_.push_back(10);

        }

    }

    for(const auto&[row, row_data]: iter::enumerate(spec.get_grid_spec()))
        for(const auto&[col, cell]: iter::enumerate(row_data))
            if(cell == AsciiLayoutSpec::LogicalComputationQubit_StandardBorderOrientation)
                cached_core_patches_.push_back(LayoutHelpers::basic_square_patch(Cell::from_ints(row,col)));

    cached_ancilla_locations_ = spec.find_all_cells_of_type(AsciiLayoutSpec::CellType::AncillaQubitLocation);

    cached_min_furthest_cell_ = spec.furthest_cell();
}


}
