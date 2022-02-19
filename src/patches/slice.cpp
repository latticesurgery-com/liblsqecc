#include <lsqecc/patches/slice.hpp>
#include <iterator>
#include <algorithm>

namespace lsqecc{


std::optional<Cell> find_place_for_magic_state(const Slice& slice, const MultipleCellsOccupiedByPatch& distillation_region)
{
    const auto& cells = distillation_region.sub_cells;
    for(const auto& cell: cells)
        for(const auto& neighbour: slice.get_neigbours_within_slice(cell.cell))
            if(slice.is_cell_free(neighbour))
                return neighbour;

    return std::nullopt;
}




Slice Slice::advance_slice() const {
    Slice new_slice{.unbound_magic_states = unbound_magic_states, .layout=layout};

    // Copy patches over
    for (const auto& old_patch : patches) {
        // Skip patches that were measured in the previous timestep
        if(old_patch.activity!=PatchActivity::Measurement)
        {
            new_slice.patches.push_back(old_patch);
            auto& new_patch = new_slice.patches.back();

            // Clear Unitary Operator activity
            if (new_patch.activity==PatchActivity::Unitary)
                new_patch.activity = PatchActivity::None;
        }
    }


    // Make magic states appear:
    for (int i = 0; i<time_to_next_magic_state_by_distillation_region.size(); ++i)
    {
        new_slice.time_to_next_magic_state_by_distillation_region.push_back(
                time_to_next_magic_state_by_distillation_region[i]-1);
        if(new_slice.time_to_next_magic_state_by_distillation_region.back() == 0){

            auto magic_state_cell = find_place_for_magic_state(new_slice, layout.distillation_regions()[i]);
            if(!magic_state_cell)
                throw std::logic_error(std::string{
                    "Could not find place for magic state produced by distillation region"} + std::to_string(i));

            Patch magic_state_patch = Layout::basic_square_patch(*magic_state_cell);
            magic_state_patch.type = PatchType::PreparedState;
            new_slice.unbound_magic_states.push_back(magic_state_patch);
            new_slice.time_to_next_magic_state_by_distillation_region.back() = layout.distillation_times()[i];
        }
    }

    return new_slice;
}

Patch& Slice::get_patch_by_id_mut(PatchId id) {
    for(auto& p: patches)
    {
        if(p.id == id)
        {
            return p;
        }
    }
    throw std::logic_error(std::string{"No patch for id: "+std::to_string(id)});
}

const Patch& Slice::get_patch_by_id(PatchId id) const {
    for(auto& p: patches)
    {
        if(p.id == id)
        {
            return p;
        }
    }
    throw std::logic_error(std::string{"No patch for id: "+std::to_string(id)});
}

Cell Slice::get_furthest_cell() const
{
    Cell ret{layout.min_furthest_cell()};
    for(const Patch& p: patches)
    {
        for (const Cell& c: p.get_cells())
        {
            ret.col = std::max(ret.col, c.col);
            ret.row = std::max(ret.row, c.row);
        }
    }
    return ret;
}


std::optional<std::reference_wrapper<const Patch>> Slice::get_patch_on_cell(const Cell& cell) const
{
    for(const Patch& p: patches)
        for (const Cell& c: p.get_cells())
            if(c==cell)
                return p;

    for(const Patch& p: unbound_magic_states)
        for (const Cell& c: p.get_cells())
            if(c==cell)
                return p;

    return std::nullopt;
}

bool Slice::is_cell_free(const Cell& cell) const
{
    for(const auto& distillation_region: layout.distillation_regions())
        for(const auto& distillation_cell: distillation_region.sub_cells)
            if(distillation_cell.cell==cell)
                return false;

    return !Slice::get_patch_on_cell(cell);
}

std::vector<Cell> Slice::get_neigbours_within_slice(const Cell& cell) const
{
    return cell.get_neigbours_within_bounding_box_inclusive({0,0},get_furthest_cell());
}

}
