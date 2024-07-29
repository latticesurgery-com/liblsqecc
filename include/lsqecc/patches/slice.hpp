#ifndef LSQECC_SLICE_HPP
#define LSQECC_SLICE_HPP

#include <lsqecc/patches/patches.hpp>
#include <lsqecc/layout/layout.hpp>

#include <queue>
#include <functional>

namespace lsqecc
{

struct Slice
{
    virtual const Layout& get_layout() const = 0;
    virtual bool has_patch(PatchId id) const = 0;
    virtual std::optional<Cell> get_cell_by_id(const PatchId id) const = 0;
    virtual bool is_cell_free(const Cell& cell) const = 0;
    virtual bool is_cell_free_or_activity(const Cell& cell, std::vector<PatchActivity> activities) const = 0;
    virtual std::vector<Cell> get_neigbours_within_slice(const Cell& cell) const = 0;
    virtual bool have_boundary_of_type_with(const Cell& target, const Cell& neighbour, PauliOperator op) const = 0;
    virtual bool is_boundary_reserved(const Cell& target, const Cell& neighbour) const = 0;
    virtual SurfaceCodeTimestep time_to_next_magic_state(size_t distillation_region_id) const = 0;

    virtual ~Slice(){};
};

}

#endif //LSQECC_SLICE_HPP
