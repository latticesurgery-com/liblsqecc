#ifndef LSQECC_PARSE_LAYOUT_SPEC_HPP
#define LSQECC_PARSE_LAYOUT_SPEC_HPP


#include <vector>
#include <string>
#include <stdexcept>

namespace lsqecc {


class LayoutSpec {
public:
    enum CellType {
        RoutingAncilla,
        LogicalComputationQubit_StandardBorderOrientation,
        AncillaQubitLocation,
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

    static LayoutSpec::CellType cell_type_from_char(char c)
    {
        switch (c)
        {
        case 'r': return LayoutSpec::CellType::RoutingAncilla;
        case 'Q': return LayoutSpec::CellType::LogicalComputationQubit_StandardBorderOrientation;
        case 'A': return LayoutSpec::CellType::AncillaQubitLocation;
        case '0': return LayoutSpec::CellType::DistillationRegion_0;
        case '1': return LayoutSpec::CellType::DistillationRegion_1;
        case '2': return LayoutSpec::CellType::DistillationRegion_2;
        case '3': return LayoutSpec::CellType::DistillationRegion_3;
        case '4': return LayoutSpec::CellType::DistillationRegion_4;
        case '5': return LayoutSpec::CellType::DistillationRegion_5;
        case '6': return LayoutSpec::CellType::DistillationRegion_6;
        case '7': return LayoutSpec::CellType::DistillationRegion_7;
        case '8': return LayoutSpec::CellType::DistillationRegion_8;
        case '9': return LayoutSpec::CellType::DistillationRegion_9;
        default:
            throw std::logic_error{"Char not in layout spec"};
        }
    }


    std::vector<std::vector<LayoutSpec::CellType>> parse(std::string_view);

};








}

#endif //LSQECC_PARSE_LAYOUT_SPEC_HPP
