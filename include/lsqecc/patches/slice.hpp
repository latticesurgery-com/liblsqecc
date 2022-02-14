#ifndef LSQECC_SLICE_HPP
#define LSQECC_SLICE_HPP

#include <lsqecc/patches/patches.hpp>

namespace lsqecc {

struct Slice {
    int32_t distance_dependant_timesteps = 1;
    std::vector<Patch> patches;
    std::vector<RoutingRegion> routing_regions;

    Slice make_copy_with_cleared_activity() const;


    Patch& get_patch_by_id(PatchId id);

    bool operator==(const Slice&) const = default;


};


}

#endif //LSQECC_SLICE_HPP
