#ifndef LSQECC_PATCHES_HPP
#define LSQECC_PATCHES_HPP


#include <lsqecc/pauli_rotations/pauli_operator.hpp>

#include <cstdint>
#include <optional>
#include <vector>
#include <variant>
#include <stdexcept>

namespace lsqecc {

enum class BoundaryType : uint8_t {
    None, // Used by routing
    Connected, // Used for multi patch
    Rough,
    Smooth,
};


BoundaryType boundary_for_operator(PauliOperator op);

struct Cell {
    using CoordinateType = int32_t;
    CoordinateType row;
    CoordinateType col;

    std::vector<Cell> get_neigbours() const;
    std::vector<Cell> get_neigbours_within_bounding_box_inclusive(const Cell& origin, const Cell& furthest_cell) const;

    template<class IntType>
    static Cell from_ints(IntType _row, IntType _col)
    {
        return Cell{static_cast<Cell::CoordinateType>(_row),Cell::CoordinateType(_col)};
    }

    bool operator==(const Cell&) const = default;
};

enum class PatchType : uint8_t {
    Distillation,
    PreparedState,
    Qubit,
    Routing
};

enum class PatchActivity : uint8_t
{
    None,
    Measurement,
    Unitary
};


struct Boundary {
    BoundaryType boundary_type;
    bool is_active;

    bool operator==(const Boundary&) const = default;
};


struct SingleCellOccupiedByPatch{
    Boundary top;
    Boundary bottom;
    Boundary left;
    Boundary right;

    std::optional<Boundary> get_boundary_with(const Cell& neighbour) const;
    bool have_boundary_of_type_with(PauliOperator op, const Cell& neighbour) const;
    std::optional<std::reference_wrapper<Boundary>> get_mut_boundary_with(const Cell& neighbour);

    Cell cell;
    bool operator==(const SingleCellOccupiedByPatch&) const = default;
};


struct MultipleCellsOccupiedByPatch {
    std::vector<SingleCellOccupiedByPatch> sub_cells;

    bool operator==(const MultipleCellsOccupiedByPatch&) const = default;
};

using PatchId = uint32_t;

struct Patch{
    // TODO perhaps this should be region?
    std::variant<SingleCellOccupiedByPatch, MultipleCellsOccupiedByPatch> cells;
    PatchType type;
    PatchActivity activity;

    std::optional<PatchId> id;

    std::vector<Cell> get_cells() const;
    const Cell& get_a_cell() const;
    bool operator==(const Patch&) const = default;
};

struct RoutingRegion
{
    std::vector<SingleCellOccupiedByPatch> cells;
    bool operator==(const RoutingRegion&) const = default;
};


PatchId make_new_patch_id();


}


#endif //LSQECC_PATCHES_HPP
