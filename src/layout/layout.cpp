#include <lsqecc/layout/layout.hpp>


namespace lsqecc {

Patch LayoutHelpers::basic_square_patch(Cell placement)
{
    return Patch{
            .cells=SingleCellOccupiedByPatch{
                    .top={BoundaryType::Rough,false},
                    .bottom={BoundaryType::Rough,false},
                    .left={BoundaryType::Smooth,false},
                    .right={BoundaryType::Smooth,false},
                    .cell=placement
            },
            .type=PatchType::Qubit,
            .id=std::nullopt,
    };
}
SingleCellOccupiedByPatch LayoutHelpers::make_distillation_region_cell(Cell placement)
{
        return SingleCellOccupiedByPatch{
                .top={BoundaryType::Connected,false},
                .bottom={BoundaryType::Connected,false},
                .left={BoundaryType::Connected,false},
                .right={BoundaryType::Connected,false},
                .cell=placement
        };
}
}