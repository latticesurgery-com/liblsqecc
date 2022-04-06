#ifndef LSQECC_SLICE_HPP
#define LSQECC_SLICE_HPP

#include <lsqecc/patches/patches.hpp>
#include <lsqecc/layout/layout.hpp>

#include <queue>
#include <functional>

namespace lsqecc {


struct Slice {
    std::vector<Patch> qubit_patches;
    std::vector<RoutingRegion> routing_regions;
    std::deque<Patch> unbound_magic_states;
    std::reference_wrapper<const Layout> layout; // reference_wrapper Keeps the slice copyable
    std::vector<SurfaceCodeTimestep> time_to_next_magic_state_by_distillation_region;


    bool has_patch(PatchId id) const;
    Patch& get_patch_by_id_mut(PatchId id);
    const Patch& get_patch_by_id(PatchId id) const;

    void delete_qubit_patch(PatchId id);

    const SingleCellOccupiedByPatch& get_single_cell_occupied_by_patch_by_id(PatchId id) const;
    SingleCellOccupiedByPatch& get_single_cell_occupied_by_patch_by_id_mut(PatchId id);
    std::optional<std::reference_wrapper<const Patch>> get_qubit_patch_on_cell(const Cell& cell) const;
    std::optional<std::reference_wrapper<const Patch>> get_magic_state_on_cell(const Cell& cell) const;
    std::optional<std::reference_wrapper<const Patch>> get_any_patch_on_cell(const Cell& cell) const;
    bool is_cell_free(const Cell& cell) const;
    std::vector<Cell> get_neigbours_within_slice(const Cell& cell) const;
    static Slice make_blank_slice(const Layout& layout);


    bool operator==(const Slice& other) const {
        return qubit_patches == other.qubit_patches
            && routing_regions == other.routing_regions
            && time_to_next_magic_state_by_distillation_region == other.time_to_next_magic_state_by_distillation_region;
    };

};


}

#endif //LSQECC_SLICE_HPP
