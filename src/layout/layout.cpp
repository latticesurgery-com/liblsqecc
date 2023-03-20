#include <lsqecc/layout/layout.hpp>


namespace lsqecc {

SparsePatch LayoutHelpers::basic_square_patch(Cell placement)
{
    return SparsePatch{
            {.type=PatchType::Qubit,
             .activity=PatchActivity::None,
             .id=std::nullopt},
            SingleCellOccupiedByPatch{
                    {.top={BoundaryType::Rough,false},
                     .bottom={BoundaryType::Rough,false},
                     .left={BoundaryType::Smooth,false},
                     .right={BoundaryType::Smooth,false}},
                    placement
            },
    };
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
LayoutHelpers::SinglePatchRotationALaLitinskiStages LayoutHelpers::single_patch_rotation_a_la_litinski(
        const SparsePatch& target_patch, const Cell& free_neighbour)
{
    if(!std::holds_alternative<SingleCellOccupiedByPatch>(target_patch.cells))
        throw std::logic_error{lstk::cat("Trying to rotate patch ", target_patch.id.value_or(-1), "which is not single cell")};

    auto target = std::get<SingleCellOccupiedByPatch>(target_patch.cells);

    RoutingRegion occupied_space;

    occupied_space.cells.emplace_back(SingleCellOccupiedByPatch{
            {.top={BoundaryType::None,false},
             .bottom={BoundaryType::None,false},
             .left={BoundaryType::None,false},
             .right={BoundaryType::None,false}},
            target.cell});
    occupied_space.cells.emplace_back(SingleCellOccupiedByPatch{
            {.top={BoundaryType::None,false},
             .bottom={BoundaryType::None,false},
             .left={BoundaryType::None,false},
             .right={BoundaryType::None,false}},
            free_neighbour});

    occupied_space.cells[0].get_mut_boundary_with(occupied_space.cells[1].cell)
        ->get() = {.boundary_type=BoundaryType::Connected, .is_active=true};
    occupied_space.cells[1].get_mut_boundary_with(occupied_space.cells[0].cell)
        ->get() = {.boundary_type=BoundaryType::Connected, .is_active=true};


   SparsePatch new_patch{target_patch};
   std::get<SingleCellOccupiedByPatch>(new_patch.cells).instant_rotate();

   return {.stage_1 = occupied_space, .stage_2 = occupied_space, .final_state = new_patch};
}

}