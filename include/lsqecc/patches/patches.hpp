#ifndef LSQECC_PATCHES_HPP
#define LSQECC_PATCHES_HPP


#include <lsqecc/pauli_rotations/pauli_operator.hpp>

#include <cstdint>
#include <optional>
#include <vector>
#include <variant>
#include <stdexcept>
#include <iostream>

namespace lsqecc {

enum class BoundaryType : uint8_t {
    None, // Used by routing
    Connected, // Used for multi patch
    Rough,
    Smooth,
    Reserved_Label1, // Used by EDPC to denote the two VDP path sets of one EDP set
    Reserved_Label2,
};

enum class CellDirection {
    top, bottom, left, right
};

CellDirection operator!(CellDirection dir);

BoundaryType operator!(BoundaryType bt);

std::ostream& operator<<(std::ostream& os, BoundaryType bt);

BoundaryType boundary_for_operator(PauliOperator op);


using OpId = uint64_t;

struct Cell {
    using CoordinateType = int32_t;
    CoordinateType row;
    CoordinateType col;

    std::vector<Cell> get_neigbours() const;
    std::vector<Cell> get_neigbours_within_bounding_box_inclusive(const Cell& origin, const Cell& furthest_cell) const;
    std::optional<Cell> get_directional_neighbor(const Cell& origin, const Cell& furthest_cell, CellDirection dir) const;

    template<class IntType>
    static Cell from_ints(IntType _row, IntType _col)
    {
        return Cell{static_cast<Cell::CoordinateType>(_row),Cell::CoordinateType(_col)};
    }

    bool operator==(const Cell&) const = default;

    bool operator<(const Cell& cell_comparison) const
    {
        return row<cell_comparison.row || (row == cell_comparison.row && col<cell_comparison.col);
    }
};

std::ostream& operator<<(std::ostream& os, const Cell& c);

enum class PatchType : uint8_t {
    Distillation,
    PreparedState,
    Qubit,
    Routing,
    Dead
};

enum class PatchActivity : uint8_t
{
    None,
    Measurement,
    Unitary,
    Distillation,
    Dead,
    MultiPatchMeasurement,
    Rotation,
    EDPC,
    Reserved
};


struct Boundary {
    BoundaryType boundary_type;
    bool is_active;

    bool operator==(const Boundary&) const = default;
};



struct CellBoundaries {
    Boundary top;
    Boundary bottom;
    Boundary left;
    Boundary right;

    bool has_active_boundary() const;

    bool operator==(const CellBoundaries&) const = default;

    void instant_rotate();
};


struct SingleCellOccupiedByPatch : public CellBoundaries {

    Cell cell;

    std::optional<Boundary> get_boundary_with(const Cell& neighbour) const;
    std::optional<Boundary*> get_boundary_reference_with(const Cell& neighbour);
    bool have_boundary_of_type_with(PauliOperator op, const Cell& neighbour) const;
    std::optional<std::reference_wrapper<Boundary>> get_mut_boundary_with(const Cell& neighbour);

    bool operator==(const SingleCellOccupiedByPatch&) const = default;
};


struct MultipleCellsOccupiedByPatch {
    std::vector<SingleCellOccupiedByPatch> sub_cells;

    bool operator==(const MultipleCellsOccupiedByPatch&) const = default;
};

using PatchId = uint32_t;

struct Patch {
    PatchType type;
    PatchActivity activity;
    std::optional<PatchId> id;
    std::optional<std::string> label;
    std::optional<OpId> operation_id;

    bool operator==(const Patch&) const = default;
};

std::ostream& operator<<(std::ostream& os, const Patch& p);


struct SparsePatch : public Patch {
    // TODO perhaps this should be region?
    std::variant<SingleCellOccupiedByPatch, MultipleCellsOccupiedByPatch> cells;

    std::vector<Cell> get_cells() const;
    const Cell& get_a_cell() const;
    bool operator==(const SparsePatch&) const = default;
    void visit_individual_cells_mut(std::function<void (SingleCellOccupiedByPatch&)> f);
    void visit_individual_cells(std::function<void (const SingleCellOccupiedByPatch&)> f) const;
    bool is_active() const;
};


struct DensePatch : public Patch {
    CellBoundaries boundaries;

    SparsePatch to_sparse_patch(const Cell& c) const;
    static DensePatch from_sparse_patch(const SparsePatch& p);

    bool is_active() const;
};


struct RoutingRegion
{
    std::vector<SingleCellOccupiedByPatch> cells;
    std::optional<OpId> routing_region_id;

    bool operator==(const RoutingRegion&) const = default;
};


PatchId make_new_patch_id();

}


#endif //LSQECC_PATCHES_HPP
