#include <lsqecc/layout/layout.hpp>


namespace lsqecc {

Patch Layout::basic_square_patch(Cell placement)
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
}