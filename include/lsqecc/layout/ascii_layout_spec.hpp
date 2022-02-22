#ifndef LSQECC_ASCII_LAYOUT_SPEC_HPP
#define LSQECC_ASCII_LAYOUT_SPEC_HPP


#include <lsqecc/layout/layout.hpp>

#include <vector>
#include <string>
#include <stdexcept>

namespace lsqecc {


class AsciiLayoutSpec {
public:
    enum CellType : char {
        RoutingAncilla = 'r',
        LogicalComputationQubit_StandardBorderOrientation = 'Q',
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
        case 'A':
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

    using CellGrid = std::vector<std::vector<AsciiLayoutSpec::CellType>> ;
    static CellGrid parse_grid(const std::string_view input);

    explicit AsciiLayoutSpec(const std::string_view input)
    : grid_spec_(parse_grid(input))
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

    LayoutFromSpec(const std::string_view spec_text)
    {
        init_cache(AsciiLayoutSpec{spec_text});
    }


    const std::vector<Patch>& core_patches() const override { return cached_core_patches_;};
    Cell min_furthest_cell() const override {return cached_min_furthest_cell_;};
    const std::vector<MultipleCellsOccupiedByPatch>& distillation_regions() const override {return cached_distillation_regions_;};
    const std::vector<SurfaceCodeTimestep>& distillation_times() const override {return cached_distillation_times_;};


private:
    std::vector<Patch> cached_core_patches_;
    Cell cached_min_furthest_cell_;
    std::vector<MultipleCellsOccupiedByPatch> cached_distillation_regions_;
    std::vector<SurfaceCodeTimestep> cached_distillation_times_;
    void init_cache(const AsciiLayoutSpec& spec);

};



}

#endif //LSQECC_ASCII_LAYOUT_SPEC_HPP
