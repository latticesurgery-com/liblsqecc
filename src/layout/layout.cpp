#include <lsqecc/layout/layout.hpp>


namespace lsqecc {

SparsePatch LayoutHelpers::basic_square_patch(Cell placement, std::optional<PatchId> id, std::optional<std::string> label)
{
    SparsePatch result{
            {.type=PatchType::Qubit,
             .activity=PatchActivity::None,
             .id=id,
            },
            SingleCellOccupiedByPatch{
                    {.top={BoundaryType::Smooth,false},
                     .bottom={BoundaryType::Smooth,false},
                     .left={BoundaryType::Rough,false},
                     .right={BoundaryType::Rough,false}},
                    placement
            },
    };
    result.label = label;
    return result;
}
SingleCellOccupiedByPatch LayoutHelpers::make_distillation_region_cell(Cell placement)
{
        return SingleCellOccupiedByPatch{
                {.top={BoundaryType::Connected,false},
                 .bottom={BoundaryType::Connected,false},
                 .left={BoundaryType::Connected,false},
                 .right={BoundaryType::Connected,false}},
                placement
        };
}

}