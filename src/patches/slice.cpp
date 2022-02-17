#include <lsqecc/patches/slice.hpp>
#include <iterator>
#include <algorithm>

namespace lsqecc{

Slice Slice::make_copy_with_cleared_activity() const {
    Slice new_slice;


    for (auto& old_patch : patches) {
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
}

const Patch& Slice::get_patch_by_id(PatchId id) const {
    for(auto& p: patches)
    {
        if(p.id == id)
        {
            return p;
        }
    }
}

Cell Slice::get_furthest_cell() const
{
    Cell ret{0,0};
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
    {
        for (const Cell& c: p.get_cells())
        {
            if(c==cell){
                return p;
            }
        }
    }
    return std::nullopt;
}


}
