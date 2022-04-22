#ifndef LSQECC_SPARSE_SLICE_HPP
#define LSQECC_SPARSE_SLICE_HPP

#include <lsqecc/layout/layout.hpp>
#include <lsqecc/layout/searchable_slice.hpp>

#include <queue>
#include <functional>

namespace lsqecc {


struct SparseSlice : public SearchableSlice{

    SparseSlice(std::vector<SparsePatch> _qubit_patches,
    std::vector<RoutingRegion> _routing_regions,
    std::deque<SparsePatch> _unbound_magic_states,
    std::reference_wrapper<const Layout> _layout, // reference_wrapper Keeps the slice copyable
    std::vector<SurfaceCodeTimestep> _time_to_next_magic_state_by_distillation_region)
        : qubit_patches(_qubit_patches),
          routing_regions(_routing_regions),
          unbound_magic_states(_unbound_magic_states),
          layout(_layout),
          time_to_next_magic_state_by_distillation_region(_time_to_next_magic_state_by_distillation_region)
    {}

    std::vector<SparsePatch> qubit_patches;
    std::vector<RoutingRegion> routing_regions;
    std::deque<SparsePatch> unbound_magic_states;
    std::reference_wrapper<const Layout> layout; // reference_wrapper Keeps the slice copyable
    std::vector<SurfaceCodeTimestep> time_to_next_magic_state_by_distillation_region;


    bool has_patch(PatchId id) const override;
    SparsePatch& get_patch_by_id_mut(PatchId id);
    const SparsePatch& get_patch_by_id(PatchId id) const;
    std::optional<Cell> get_cell_by_id(PatchId id) const override;

    void delete_qubit_patch(PatchId id);

    const SingleCellOccupiedByPatch& get_single_cell_occupied_by_patch_by_id(PatchId id) const;
    SingleCellOccupiedByPatch& get_single_cell_occupied_by_patch_by_id_mut(PatchId id);
    std::optional<std::reference_wrapper<const SparsePatch>> get_qubit_patch_on_cell(const Cell& cell) const;
    std::optional<std::reference_wrapper<const SparsePatch>> get_magic_state_on_cell(const Cell& cell) const;
    std::optional<std::reference_wrapper<const SparsePatch>> get_any_patch_on_cell(const Cell& cell) const;
    bool is_cell_free(const Cell& cell) const override;
    std::vector<Cell> get_neigbours_within_slice(const Cell& cell) const override;
    bool have_boundary_of_type_with(const Cell& target, const Cell& neighbour, PauliOperator op) const override;


    Cell furthest_cell() const override;

    static SparseSlice make_blank_slice(const Layout& layout);


    bool operator==(const SparseSlice& other) const {
        return qubit_patches == other.qubit_patches
            && routing_regions == other.routing_regions
            && time_to_next_magic_state_by_distillation_region == other.time_to_next_magic_state_by_distillation_region;
    };

};


}

#endif //LSQECC_SPARSE_SLICE_HPP
