#ifndef LSQECC_SEARCHABLE_SLICE_HPP
#define LSQECC_SEARCHABLE_SLICE_HPP


#include <lsqecc/patches/patches.hpp>
#include <lsqecc/layout/layout.hpp>

#include <queue>
#include <functional>

namespace lsqecc {


struct SearchableSlice
{
    virtual bool has_patch(PatchId id) const = 0;
    virtual std::optional<Cell> get_cell_by_id(const PatchId id) const = 0;
    virtual bool is_cell_free(const Cell& cell) const = 0;
    virtual Cell furthest_cell() const = 0;
    virtual std::vector<Cell> get_neigbours_within_slice(const Cell& cell) const = 0;
    virtual bool have_boundary_of_type_with(const Cell& target, const Cell& neighbour, PauliOperator op) const = 0;


    virtual ~SearchableSlice(){};
};


}


#endif //LSQECC_SEARCHABLE_SLICE_HPP
