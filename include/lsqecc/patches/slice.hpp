#ifndef LSQECC_SLICE_HPP
#define LSQECC_SLICE_HPP

#include <lsqecc/patches/patches.hpp>

namespace lsqecc {

struct Slice {
    int32_t distance_dependant_timesteps = 1;
    std::vector<Patch> patches;
    std::vector<RoutingRegion> routing_regions;
    Cell min_furthest_cell = {1,1};

    Slice make_copy_with_cleared_activity() const;

    Cell get_furthest_cell() const;

    Patch& get_patch_by_id_mut(PatchId id);
    const Patch& get_patch_by_id(PatchId id) const;
    std::optional<std::reference_wrapper<const Patch>> get_patch_on_cell(const Cell& cell) const;

    bool operator==(const Slice&) const = default;


};


}

#endif //LSQECC_SLICE_HPP
