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
LayoutHelpers::SinglePatchRotationALaLitinskiStages LayoutHelpers::single_patch_rotation_a_la_litinski(
        const Patch& target_patch, const Cell& free_neighbour)
{
    if(!std::holds_alternative<SingleCellOccupiedByPatch>(target_patch.cells))
        throw std::logic_error{lstk::cat("Trying to rotate patch ", target_patch.id.value_or(-1), "which is not single cell")};

    auto target = std::get<SingleCellOccupiedByPatch>(target_patch.cells);

    RoutingRegion occupied_space;

    occupied_space.cells.emplace_back(SingleCellOccupiedByPatch{
            .top={BoundaryType::None,false},
            .bottom={BoundaryType::None,false},
            .left={BoundaryType::None,false},
            .right={BoundaryType::None,false},
            .cell=target.cell});
    occupied_space.cells.emplace_back(SingleCellOccupiedByPatch{
            .top={BoundaryType::None,false},
            .bottom={BoundaryType::None,false},
            .left={BoundaryType::None,false},
            .right={BoundaryType::None,false},
            .cell=free_neighbour});

    occupied_space.cells[0].get_mut_boundary_with(occupied_space.cells[1].cell)
        ->get() = {.boundary_type=BoundaryType::Connected, .is_active=true};
    occupied_space.cells[1].get_mut_boundary_with(occupied_space.cells[0].cell)
        ->get() = {.boundary_type=BoundaryType::Connected, .is_active=true};


   SingleCellOccupiedByPatch rotated_first_patch = target;
   for(const auto cell : target.cell.get_neigbours())
   {
       const BoundaryType& original_boundary_type = target.get_boundary_with(cell)->boundary_type;
       Boundary& new_boundary = rotated_first_patch.get_mut_boundary_with(cell)->get();
       if(original_boundary_type==BoundaryType::Rough)
           new_boundary.boundary_type = BoundaryType::Smooth;
       else if(original_boundary_type==BoundaryType::Smooth)
           new_boundary.boundary_type = BoundaryType::Rough;
       else
           throw std::logic_error(lstk::cat("Trying to rotate patch at ",target.cell.row,", ",target.cell.col,
                   " with unexpected boundary type ", static_cast<int64_t>(original_boundary_type)));

       new_boundary.is_active = false;
   }

   Patch new_patch{target_patch};
   new_patch.cells = rotated_first_patch;

   return {.stage_1 = occupied_space, .stage_2 = occupied_space, .final_state = new_patch};
}

}