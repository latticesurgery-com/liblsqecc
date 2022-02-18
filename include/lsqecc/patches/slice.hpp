#ifndef LSQECC_SLICE_HPP
#define LSQECC_SLICE_HPP

#include <lsqecc/patches/patches.hpp>
#include <lsqecc/layout/layout.hpp>

namespace lsqecc {


struct Slice {
    std::vector<Patch> patches;
    std::vector<RoutingRegion> routing_regions;
    const Layout& layout;
    std::vector<SurfaceCodeTimestep> time_to_next_magic_state_by_distillation_region;


    Slice make_copy_with_cleared_activity() const;

    Cell get_furthest_cell() const;

    Patch& get_patch_by_id_mut(PatchId id);
    const Patch& get_patch_by_id(PatchId id) const;
    std::optional<std::reference_wrapper<const Patch>> get_patch_on_cell(const Cell& cell) const;

    bool operator==(const Slice& other) const {
        return patches == other.patches
            && routing_regions == other.routing_regions
            && time_to_next_magic_state_by_distillation_region == other.time_to_next_magic_state_by_distillation_region;
    };

};


}

#endif //LSQECC_SLICE_HPP
