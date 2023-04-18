#ifndef LSQECC_ASCII_LAYOUT_SPEC_HPP
#define LSQECC_ASCII_LAYOUT_SPEC_HPP


#include <lsqecc/layout/layout.hpp>

#include <vector>
#include <string>
#include <array>
#include <stdexcept>

namespace lsqecc {


class AsciiLayoutSpec {
public:
    enum CellType : char {
        RoutingAncilla = 'r',
        LogicalComputationQubit_StandardBorderOrientation = 'Q',
        LogicalComputationQubit_RotatedBorderOrientation = 'T',
        LogicalComputationQubit_DynamicAllocation = 'D',
        AncillaQubitLocation = 'A',
        DistillationRegion_0 = '0',
        DistillationRegion_1 = '1',
        DistillationRegion_2 = '2',
        DistillationRegion_3 = '3',
        DistillationRegion_4 = '4',
        DistillationRegion_5 = '5',
        DistillationRegion_6 = '6',
        DistillationRegion_7 = '7',
        DistillationRegion_8 = '8',
        DistillationRegion_9 = '9',
        ReservedForMagicState = 'M',
        DeadCell = 'X',
        PreDistilledYState = 'Y'
    };

    static constexpr std::array<CellType,10> k_distillation_region_types = {
            DistillationRegion_0,
            DistillationRegion_1,
            DistillationRegion_2,
            DistillationRegion_3,
            DistillationRegion_4,
            DistillationRegion_5,
            DistillationRegion_6,
            DistillationRegion_7,
            DistillationRegion_8,
            DistillationRegion_9,
    };

    static AsciiLayoutSpec::CellType cell_type_from_char(char c)
    {
        switch (c)
        {
        case 'r':
        case 'Q':
        case 'T':
        case 'D':
        case 'A':
        case 'M':
        case 'X':
        case 'Y':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return static_cast<AsciiLayoutSpec::CellType>(c);
        default:
            throw std::logic_error{"Char not in layout spec"};
        }
    }

    using CellRow = std::vector<AsciiLayoutSpec::CellType> ;
    using CellGrid = std::vector<CellRow> ;
    static CellGrid parse_grid(const std::string_view input);

    explicit AsciiLayoutSpec(const std::string_view input)
    : grid_spec_(parse_grid(input))
    {}

    explicit AsciiLayoutSpec(const CellGrid& grid_spec)
            : grid_spec_(grid_spec)
    {}


    std::optional<Cell> find_a_cell_of_type(AsciiLayoutSpec::CellType target) const;
    std::vector<Cell> find_all_cells_of_type(AsciiLayoutSpec::CellType target) const;
    std::vector<Cell> connected_component(Cell start) const;
    Cell furthest_cell() const;

    std::ostream &operator<<(std::ostream &os);

    const CellGrid& get_grid_spec() const {return grid_spec_;};
private:
    CellGrid grid_spec_;
};



class LayoutFromSpec : public Layout{
public:
    explicit LayoutFromSpec(const std::string_view spec_text, const DistillationOptions& distillation_options)
    {
        init_cache(AsciiLayoutSpec{spec_text}, distillation_options);
    }

    explicit LayoutFromSpec(const AsciiLayoutSpec::CellGrid& grid_spec, const DistillationOptions& distillation_options)
    {
        init_cache(AsciiLayoutSpec{grid_spec}, distillation_options);
    }


    const std::vector<SparsePatch>& core_patches() const override { return cached_core_patches_;};
    Cell furthest_cell() const override {return cached_furthest_cell_;};
    const std::vector<MultipleCellsOccupiedByPatch>& distillation_regions() const override {return cached_distillation_regions_;};
    const std::vector<SurfaceCodeTimestep>& distillation_times() const override {return cached_distillation_times_;};
    const std::vector<Cell>& ancilla_location() const override {return cached_ancilla_locations_;}
    const std::vector<Cell>& dead_location() const override {return cached_dead_cells_;}
    const std::vector<Cell>& y_states() const override {return cached_y_states_;}
    const std::vector<Cell>& distilled_state_locations(size_t distillation_region_idx) const override
    {
        return cached_distilled_state_locations_[distillation_region_idx];
    }
    const bool magic_states_reserved() const override {return magic_states_reserved_;}

private:
    std::vector<SparsePatch> cached_core_patches_;
    std::vector<Cell> cached_y_states_;
    Cell cached_furthest_cell_;
    std::vector<MultipleCellsOccupiedByPatch> cached_distillation_regions_;
    std::vector<SurfaceCodeTimestep> cached_distillation_times_;
    std::vector<Cell> cached_ancilla_locations_;
    std::vector<std::vector<Cell>> cached_distilled_state_locations_;
    std::vector<Cell> cached_dead_cells_;
    bool magic_states_reserved_; 
    void init_cache(const AsciiLayoutSpec& spec, const DistillationOptions& distillation_options);

};

}

#endif //LSQECC_ASCII_LAYOUT_SPEC_HPP
