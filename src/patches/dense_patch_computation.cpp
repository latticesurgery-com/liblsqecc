#include <lsqecc/patches/dense_patch_computation.hpp>
#include <lsqecc/dag/domain_dags.hpp>
#include <lsqecc/scheduler/wave_scheduler.hpp>

#include <algorithm>
#include <sstream>
#include <iterator> 

namespace lsqecc
{


BusyRegion single_patch_rotation_a_la_litinski(
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

   return {.region = occupied_space, .steps_to_clear=3, .state_after_clearing = {new_patch}};
}


std::optional<Cell> find_place_for_magic_state(const DenseSlice& slice, const Layout& layout, size_t distillation_region_idx)
{
    for(const auto& cell: layout.distilled_state_locations(distillation_region_idx))
    {
        if(slice.is_cell_free(cell))
            return cell;
    }
    return std::nullopt;
}

std::optional<Cell> find_free_ancilla_location(const Layout& layout, const DenseSlice& slice)
{
    for(const Cell& possible_ancilla_location : layout.ancilla_location())
        if(slice.is_cell_free(possible_ancilla_location))
            return possible_ancilla_location;
    return std::nullopt;
}

std::optional<Cell> place_ancilla_next_to(const DenseSlice& slice, PatchId target, PauliOperator boundary_op)
{
    Cell target_cell = slice.get_cell_by_id(target).value();
    for(const Cell& possible_ancilla_location : slice.get_neigbours_within_slice(target_cell))
    {
        auto boundary = slice.get_boundary_between(target_cell, possible_ancilla_location);
        if(boundary && boundary->get().boundary_type == boundary_for_operator(boundary_op) && slice.is_cell_free(possible_ancilla_location))
            return possible_ancilla_location;
    }
    return std::nullopt;
}

void advance_slice(DenseSlice& slice, const Layout& layout)
{
    slice.traverse_cells_mut([&](const Cell& c, std::optional<DensePatch>& p) {
        if(!p) return;
        if(p->activity == PatchActivity::Unitary)
            p->activity = PatchActivity::None;

        if (p->activity == PatchActivity::EDPC)
        {
            p->activity = PatchActivity::Reserved;
            // p->boundaries.top.boundary_type = BoundaryType::Rough;
            // p->boundaries.right.boundary_type = BoundaryType::Smooth;
            // p->boundaries.bottom.boundary_type = BoundaryType::Rough;
            // p->boundaries.left.boundary_type = BoundaryType::Smooth;
        }

        if ((p->activity != PatchActivity::Rotation) && (p->activity != PatchActivity::Reserved))
        {
            p->boundaries.top.is_active = false;
            p->boundaries.bottom.is_active = false;
            p->boundaries.left.is_active = false;
            p->boundaries.right.is_active = false;
        }

        if((p->activity == PatchActivity::MultiPatchMeasurement) || p->activity == PatchActivity::Measurement)
        {
            p = std::nullopt;
            return;
        }
    });

    // If we have tiles reserved for magic state re-spawn, we loop over them and 
    //  * If a state was consumed in the last slice, we reset the tile and the re-spawn time
    //  * Else, if we are waiting for re-spawn, we decrement the re-spawn time
    //  * If it is time to re-spawn, we add a PreparedState.
    if (layout.magic_states_reserved()) 
    {
        size_t reserved_cell_index = 0;
        for (const Cell& cell: layout.reserved_for_magic_states()) {
            if (slice.is_cell_free(cell)) 
            {
                slice.patch_at(cell) = DensePatch{
                    Patch{PatchType::Distillation,PatchActivity::Distillation,std::nullopt},
                    CellBoundaries{Boundary{BoundaryType::Connected, false},Boundary{BoundaryType::Connected, false},
                        Boundary{BoundaryType::Connected, false},Boundary{BoundaryType::Connected, false}}};
                slice.time_to_next_magic_state_by_distillation_region[reserved_cell_index] = layout.distillation_times()[reserved_cell_index];
            }
            else if (slice.patch_at(cell)->type == PatchType::Distillation)
            {
                slice.time_to_next_magic_state_by_distillation_region[reserved_cell_index]--;
            }

            if (slice.time_to_next_magic_state_by_distillation_region[reserved_cell_index] == 0)
            {
                if ((slice.patch_at(cell)->type != PatchType::PreparedState) &&
                    (slice.patch_at(cell)->type != PatchType::Qubit))
                {
                    SparsePatch magic_state_patch = LayoutHelpers::basic_square_patch(cell, std::nullopt, "Magic State");
                    magic_state_patch.type = PatchType::PreparedState;
                    slice.place_single_cell_sparse_patch(magic_state_patch, true);
                    slice.magic_states.insert(cell); 
                }
            }
            reserved_cell_index++;
        }            
    }

    // Otherwise, we update the factories
    else
    {
        size_t distillation_region_index = 0;
        for (auto& time_to_magic_state_here: slice.time_to_next_magic_state_by_distillation_region)
        {        
            time_to_magic_state_here--;

            if(time_to_magic_state_here == 0){

                auto magic_state_cell = find_place_for_magic_state(slice, layout, distillation_region_index);
                if(magic_state_cell)
                {
                    SparsePatch magic_state_patch = LayoutHelpers::basic_square_patch(*magic_state_cell, std::nullopt, "Magic State");
                    magic_state_patch.type = PatchType::PreparedState;
                    slice.place_single_cell_sparse_patch(magic_state_patch, true);
                    slice.magic_states.insert(*magic_state_cell);
                }
                time_to_magic_state_here = layout.distillation_times()[distillation_region_index];
            }

            distillation_region_index++;
        }
    }

    // Reset EDPC crossing vertices
    slice.EDPC_crossing_vertices.clear();

}


void stitch_boundaries(
        DenseSlice& slice,
        Cell source_cell,
        Cell target_cell,
        RoutingRegion& routing_region)
{
    if (routing_region.cells.empty())
    {
        slice.get_boundary_between(source_cell,target_cell)->get().is_active=true;
        slice.get_boundary_between(target_cell,source_cell)->get().is_active=true;
    }
    else
    {
        slice.get_boundary_between(source_cell,routing_region.cells.back().cell)->get().is_active=true;
        slice.get_boundary_between(target_cell,routing_region.cells.front().cell)->get().is_active=true;
    }
}

void mark_routing_region(DenseSlice& slice, RoutingRegion& routing_region, PatchActivity activity)
{

    // For purpose of EDPC
    BoundaryType current_label = BoundaryType::Reserved_Label1;

    // Loop over cells in routing region
    size_t counter = 0;
    Cell* prev_cell = nullptr;
    for (auto& occupied_cell : routing_region.cells)
    {
        
        // Obtain patch at current cell if one exists
        std::optional<DensePatch>& patch = slice.patch_at(occupied_cell.cell);

        // In the case that there is a patch allocated to this cell,
        if (patch.has_value())
        {
            // In the case that it has already been a part of EDPC (implying that we have found a crossing path),
            if (patch->activity == PatchActivity::EDPC && (activity == PatchActivity::EDPC))
            {

                // If this crossing vertex has not been yet seen, add it to the set (and mark it with a measurement for visualization purposes)
                auto result = slice.EDPC_crossing_vertices.insert(occupied_cell.cell);

                if (!result.second) 
                {
                    throw std::runtime_error("Vertices cannot be crossed by more than two paths in EDPC.");
                }

                // Update boundaries of patch at crossing vertex, making sure that the new edges have the opposite label as the old edges
                current_label = slice.mark_boundaries_for_crossing_cell(patch.value(), occupied_cell, *prev_cell);
            }

            // Otherwise, we check whether this is the control or target qubit and add appropriate boundaries if so
            else if (activity == PatchActivity::EDPC)
            {
                if (counter == 0)
                {
                    auto bdry = slice.get_boundary_between(occupied_cell.cell, routing_region.cells[1].cell);
                    if (bdry.has_value()) {
                        bdry.value().get().boundary_type = current_label; 
                        slice.marked_rough_boundaries_EDPC.push_back(bdry.value());
                    }
                    else
                        throw std::runtime_error("Route invalid for target qubit in EDPC.");

                }

                else if (counter == routing_region.cells.size() - 1)
                {
                    auto bdry = slice.get_boundary_between(occupied_cell.cell, routing_region.cells[routing_region.cells.size()-2].cell);
                    if (bdry.has_value()) {
                        bdry.value().get().boundary_type = current_label; 
                        slice.marked_smooth_boundaries_EDPC.push_back(bdry.value());   
                    }
                    else
                        throw std::runtime_error("Route invalid for control qubit in EDPC.");
                }

                else
                {
                    throw std::runtime_error("Attempting to route through a cell that already contains a patch."); 
                }
            }

            else
            {
                throw std::runtime_error("Attempting to route through a cell that already contains a patch."); 
            }
        }

        else
        {
            if (activity == PatchActivity::EDPC)
            {
                // If a patch has not already been assigned, we assume that we can assign it to VDP set 1 through BoundaryType_ReservedLabel1
                std::vector<Boundary*> boundary_ptrs = {&occupied_cell.top, &occupied_cell.bottom, &occupied_cell.left, &occupied_cell.right};
                for (Boundary* bdry : boundary_ptrs)
                {
                    if ((bdry->boundary_type != BoundaryType::None) && (bdry->boundary_type != current_label))
                        bdry->boundary_type = current_label;
                }
            }

            SparsePatch tmp = SparsePatch{{PatchType::Routing, activity}, occupied_cell};
            tmp.operation_id = routing_region.routing_region_id;
            slice.place_sparse_patch(tmp, false);
        }

        counter++;
        prev_cell = &occupied_cell.cell;
    }
}

void clear_routing_region(DenseSlice& slice, const RoutingRegion& routing_region)
{
    for (const auto& occupied_cell : routing_region.cells)
    {
        auto cell = occupied_cell.cell;
        auto& patch = slice.patch_at(cell);
        assert(patch && patch->type == PatchType::Routing);
        patch = std::nullopt;
    }
}


static OpId multi_body_measurement_op_id_counter = 1;

/*
 * Returns true iff merge was successful
 */
bool merge_patches(
        DenseSlice& slice,
        Router& router,
        PatchId source,
        PauliOperator source_op,
        PatchId target,
        PauliOperator target_op,
        bool gen_op_ids)
{

    // TODO remove duplicate cell/patch search
    if(slice.get_patch_by_id(source)->get().is_active() || slice.get_patch_by_id(target)->get().is_active())
    {
        return false;
    }

    auto routing_region = router.find_routing_ancilla(slice, source, source_op, target, target_op);
    if(!routing_region)
    {
        return false;
    }

    if (gen_op_ids)
        routing_region->routing_region_id = multi_body_measurement_op_id_counter++;

    // TODO check that the path is actually free when caching

    stitch_boundaries(slice, *slice.get_cell_by_id(source), *slice.get_cell_by_id(target), *routing_region);
    mark_routing_region(slice, *routing_region, PatchActivity::MultiPatchMeasurement);

    return true;
}


InstructionApplicationResult try_apply_local_instruction(
        DenseSlice& slice,
        LocalInstruction::LocalLSInstruction instruction,
        const Layout& layout,
        Router& router)
{
        if (const auto* prepy = std::get_if<LocalInstruction::PrepareY>(&instruction.operation))
    {
        if (!slice.is_cell_free(prepy->target_cell))
        {
            throw std::runtime_error(lstk::cat(instruction, "; Cell ", prepy->target_cell, " is not free, cannot prepare state"));
        }

        slice.place_single_cell_sparse_patch(LayoutHelpers::basic_square_patch(prepy->target_cell, prepy->target_id, "Y State"), false);

        // PatchActivity::Unitary may or may not be appropriate here
        slice.patch_at(prepy->target_cell).value().activity = PatchActivity::Unitary;

        return {nullptr, {}};
    }

    else if (const auto* bellprep = std::get_if<LocalInstruction::BellPrepare>(&instruction.operation))
    {

        if (router.get_EDPC())
        {
            if (!slice.is_cell_free_or_activity(bellprep->cell1, {PatchActivity::EDPC, PatchActivity::Reserved}))
            {
                return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Cell ", bellprep->cell1, " is not free, cannot prepare state")), {}};
            }
            else if (!slice.is_cell_free_or_activity(bellprep->cell2, {PatchActivity::EDPC, PatchActivity::Reserved}))
            {
                return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Cell ", bellprep->cell2, " is not free, cannot prepare state")), {}};
            }
            slice.cells.at(bellprep->cell1.row).at(bellprep->cell1.col) = std::nullopt;
            slice.cells.at(bellprep->cell2.row).at(bellprep->cell2.col) = std::nullopt;
        }

        else
        {
            if (!slice.is_cell_free(bellprep->cell1))
            {
                return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Cell ", bellprep->cell1, " is not free, cannot prepare state")), {}};
            }
            else if (!slice.is_cell_free(bellprep->cell2))
            {
                return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Cell ", bellprep->cell2, " is not free, cannot prepare state")), {}};
            }
        }

        slice.place_single_cell_sparse_patch(LayoutHelpers::basic_square_patch(bellprep->cell1, bellprep->side1, "Bell 1"), false);
        slice.place_single_cell_sparse_patch(LayoutHelpers::basic_square_patch(bellprep->cell2, bellprep->side2, "Bell 2"), false);
        slice.get_boundary_between_or_fail(bellprep->cell1, bellprep->cell2).get().is_active=true;
        slice.get_boundary_between_or_fail(bellprep->cell2, bellprep->cell1).get().is_active=true;

        return {nullptr, {}};
    }

    else if (const auto* bellmeas = std::get_if<LocalInstruction::BellMeasure>(&instruction.operation))
    {
        if (!slice.patch_at(bellmeas->cell1).has_value())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; No Patch at ", bellmeas->cell1, ", cannot measure")), {}};
        else if (!slice.patch_at(bellmeas->cell2).has_value())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; No Patch at ", bellmeas->cell2, ", cannot measure")), {}};
        else if (slice.patch_at(bellmeas->cell1)->is_active())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Patch at ", bellmeas->cell1, " is active, cannot measure")), {}};
        else if (slice.patch_at(bellmeas->cell2)->is_active())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Patch at ", bellmeas->cell2, " is active, cannot measure")), {}};

        slice.get_boundary_between_or_fail(bellmeas->cell1,bellmeas->cell2).get().is_active=true;
        slice.get_boundary_between_or_fail(bellmeas->cell2,bellmeas->cell1).get().is_active=true;
        slice.patch_at(bellmeas->cell1).value().activity = PatchActivity::Measurement;
        slice.patch_at(bellmeas->cell2).value().activity = PatchActivity::Measurement;

        return {nullptr, {}};
    }

    else if (const auto* move = std::get_if<LocalInstruction::Move>(&instruction.operation))
    {

        if (!slice.patch_at(move->source_cell).has_value())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; No Patch at ", move->source_cell, ", cannot move")), {}};
        else if (slice.patch_at(move->source_cell)->is_active())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Patch at ", move->source_cell, " is active, cannot move")), {}};

        if (router.get_EDPC())
        {
            if (!slice.is_cell_free_or_activity(move->target_cell, {PatchActivity::EDPC, PatchActivity::Reserved}))
            {
                return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Cell ", move->target_cell, " is not free, cannot move")), {}};        
            }

            else
            {
                slice.cells.at(move->target_cell.row).at(move->target_cell.col) = std::nullopt;
            }
        }

        else 
        {
            if (!slice.is_cell_free(move->target_cell))
                return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Cell ", move->target_cell, " is not free, cannot move")), {}};
        }
        
        SparsePatch new_patch = LayoutHelpers::basic_square_patch(move->target_cell, std::nullopt, "Move");
        new_patch.id = move->new_id_for_target ? move->new_id_for_target : slice.patch_at(move->source_cell)->id;
        slice.place_single_cell_sparse_patch(new_patch, false);
        slice.get_boundary_between_or_fail(move->source_cell, move->target_cell).get().is_active=true;
        slice.get_boundary_between_or_fail(move->target_cell, move->source_cell).get().is_active=true;
        slice.patch_at(move->source_cell)->activity = PatchActivity::Measurement;
        slice.patch_at(move->source_cell)->id = std::nullopt;

        return {nullptr, {}};
    }

    else if (const auto* localmeas = std::get_if<LocalInstruction::TwoPatchMeasure>(&instruction.operation))
    {
        if (!slice.patch_at(localmeas->cell1).has_value())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; No Patch at ", localmeas->cell1, ", cannot measure")), {}};
        else if (!slice.patch_at(localmeas->cell2).has_value())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; No Patch at ", localmeas->cell2, ", cannot measure")), {}};
        else if (slice.patch_at(localmeas->cell1)->is_active())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Patch at ", localmeas->cell1, " is active, cannot measure")), {}};
        else if (slice.patch_at(localmeas->cell2)->is_active())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Patch at ", localmeas->cell2, " is active, cannot measure")), {}};

        slice.get_boundary_between_or_fail(localmeas->cell1,localmeas->cell2).get().is_active=true;
        slice.get_boundary_between_or_fail(localmeas->cell2,localmeas->cell1).get().is_active=true;

        return {nullptr, {}};
    }

    else if (const auto* extendsplit = std::get_if<LocalInstruction::ExtendSplit>(&instruction.operation))
    {

        if (!slice.patch_at(extendsplit->target_cell).has_value())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; No Patch at ", extendsplit->target_cell, ", cannot extend")), {}};
        else if (slice.patch_at(extendsplit->target_cell)->is_active())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Patch at ", extendsplit->target_cell, " is active, cannot extend")), {}};

        if (router.get_EDPC())
        {
            if (!slice.is_cell_free_or_activity(extendsplit->extension_cell, {PatchActivity::EDPC, PatchActivity::Reserved}))
            {
                return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Cell ", extendsplit->extension_cell, " is not free or marked for EDPC, cannot extend")), {}};        
            }

            else
            {
                slice.cells.at(extendsplit->extension_cell.row).at(extendsplit->extension_cell.col) = std::nullopt;
            }
        }

        else 
        {
            if (!slice.is_cell_free(extendsplit->extension_cell))
                return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Cell ", extendsplit->extension_cell, " is not free, cannot extend")), {}};
        }

        slice.place_single_cell_sparse_patch(LayoutHelpers::basic_square_patch(extendsplit->extension_cell, extendsplit->extension_id, "Extended"), false);
        slice.get_boundary_between_or_fail(extendsplit->extension_cell, extendsplit->target_cell).get().is_active=true;
        slice.get_boundary_between_or_fail(extendsplit->target_cell, extendsplit->extension_cell).get().is_active=true;

        return {nullptr, {}};
    }

    else if (const auto* mergecont = std::get_if<LocalInstruction::MergeContract>(&instruction.operation))
    {
        if (!slice.patch_at(mergecont->preserved_cell).has_value())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; No Patch at ", mergecont->preserved_cell, ", cannot merge")), {}};
        else if (!slice.patch_at(mergecont->measured_cell).has_value())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; No Patch at ", mergecont->measured_cell, ", cannot merge")), {}};
        else if (slice.patch_at(mergecont->preserved_cell)->is_active())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Patch at ", mergecont->preserved_cell, " is active, cannot merge")), {}};
        else if (slice.patch_at(mergecont->measured_cell)->is_active())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction, "; Patch at ", mergecont->measured_cell, " is active, cannot merge")), {}};

        slice.get_boundary_between_or_fail(mergecont->preserved_cell,mergecont->measured_cell).get().is_active=true;
        slice.get_boundary_between_or_fail(mergecont->measured_cell,mergecont->preserved_cell).get().is_active=true;
        slice.patch_at(mergecont->measured_cell).value().activity = PatchActivity::Measurement;

        return {nullptr, {}};
    }
    LSTK_UNREACHABLE;
}

// Enables specification of long-range ops: Bell preparation, Bell measurement, Bell-mediated merge, Bell-mediated split
enum class LongRangeBell {
    Prep, Measure, Merge, Split
};

/* Compile long-range Bell-based operations: Bell preparation, Bell measurement, Bell-mediated merge, Bell-mediated split.
    - Appends two layers of local instructions to input vectors layer1 and layer2
    - Local instructions are applied along a RoutingRegion segment defined by route, start, and end
    - Directionality can be reversed: reverse==false implies Merge/Split occurs at source-end and Moves occur at target-end
    - PatchIDs can be given for either end of any resultant long-range Bell state
*/
void compile_long_range_Bell(LongRangeBell type, std::vector<LocalInstruction::LocalLSInstruction>& layer1, std::vector<LocalInstruction::LocalLSInstruction>& layer2, const RoutingRegion& route, size_t start, size_t end, 
                                    bool reverse = false, std::optional<PatchId> side1 = std::nullopt, std::optional<PatchId> side2 = std::nullopt)
{
    // Check input validity
    if (start >= end || end > route.cells.size()) {
        throw std::invalid_argument("Invalid segment range");
    }

    // Ascertain whether route contains an even or odd number of cells
    size_t route_size = end - start + 1;
    bool even_route = ((route_size)%2 == 0);

    // If we are performing a Bell measurement or a Bell-mediated split, a Move operation goes in the second layer
    if (((type == LongRangeBell::Prep) || (type == LongRangeBell::Split)) && (!even_route))
    {
        // The Move operation is by default located at the end of the range, but if `reverse', it is located at the source-end (beginning)
        if (!reverse)
        {
            layer2.push_back({LocalInstruction::Move{
                route.cells[start+1].cell, 
                route.cells[start].cell,
                std::nullopt
            }}); 
        }

        else
        {
            layer2.push_back({LocalInstruction::Move{
                route.cells[end-1].cell, 
                route.cells[end].cell,
                std::nullopt
            }});      
        }
    }

    // LongRangeBell::Split is an ExtendSplit on the source qubit (end of range), a long-range Bell preparation, and a Bell measure between them
    if (type == LongRangeBell::Split)
    {
        if (((route_size == 2) || (route_size == 3)) && !reverse)
        {
            layer1.push_back({LocalInstruction::ExtendSplit{
                std::nullopt,
                route.cells[end].cell,
                route.cells[end-1].cell
            }});

            return;
        }

        else if (((route_size == 2) || (route_size == 3)) && reverse)
        {
            layer1.push_back({LocalInstruction::ExtendSplit{
                std::nullopt,
                route.cells[start].cell,
                route.cells[start+1].cell
            }});  

            return;    
        }

        else if (!reverse)
        {
            layer1.push_back({LocalInstruction::ExtendSplit{
                std::nullopt,
                route.cells[end].cell,
                route.cells[end-1].cell
            }});

            layer2.push_back({LocalInstruction::BellMeasure{
                route.cells[end-1].cell,
                route.cells[end-2].cell
            }});

           end = end - 2;
        }

        else 
        {
            layer1.push_back({LocalInstruction::ExtendSplit{
                std::nullopt,
                route.cells[start].cell,
                route.cells[start+1].cell
            }});

            layer2.push_back({LocalInstruction::BellMeasure{
                route.cells[start+1].cell,
                route.cells[start+2].cell
            }});

            start = start + 2;
        }
    }

    // If we are performing a Bell Measurement or a Bell-mediated merge, a Move operation goes in the first layer
    if (((type == LongRangeBell::Measure) || (type == LongRangeBell::Merge)) && (!even_route))
    {
        // The Move operation is by default located at the end of the range, but if `reverse', it is located at the source-end (beginning)
        if (!reverse)
        {
            layer1.push_back({LocalInstruction::Move{
                route.cells[start].cell, 
                route.cells[start+1].cell,
                std::nullopt
            }}); 
        }

        else
        {
            layer1.push_back({LocalInstruction::Move{
                route.cells[end].cell, 
                route.cells[end-1].cell,
                std::nullopt
            }});      
        }
    }

    // LongRangeBell::Merge is a Bell state preparation, a MergeContract of one side with the source qubit, and a BellMeasure of the other side with the target
    if (type == LongRangeBell::Merge)
    {
        if (((route_size == 2) || (route_size == 3)) && !reverse)
        {
            layer2.push_back({LocalInstruction::MergeContract{
                route.cells[end].cell,
                route.cells[end-1].cell
            }});

            return;
        }

        else if (((route_size == 2) || (route_size == 3)) && reverse)
        {
            layer2.push_back({LocalInstruction::MergeContract{
                route.cells[start].cell,
                route.cells[start+1].cell
            }});

            return;   
        }

        else if (!reverse)
        {
            layer1.push_back({LocalInstruction::BellPrepare{
                side1,
                std::nullopt,
                route.cells[end-1].cell,
                route.cells[end-2].cell
            }});

            layer2.push_back({LocalInstruction::MergeContract{
                route.cells[end].cell,
                route.cells[end-1].cell
            }});

            end = end - 2;
        }

        else 
        {
            layer1.push_back({LocalInstruction::BellPrepare{
                side2,
                std::nullopt,
                route.cells[start+1].cell,
                route.cells[start+2].cell
            }});

            layer2.push_back({LocalInstruction::MergeContract{
                route.cells[start].cell,
                route.cells[start+1].cell
            }});    

            start = start + 2;       
        }
    } 

    // Push a layer of BellPrepare instructions followed by a layer of BellMeasure instructions
    /* If the route is even, the compilation is invariant to the value of 'reverse'. Otherwise, we may offset the starting point to accomodate Move placement at beginning or end. */
    std::optional<PatchId> id1; std::optional<PatchId> id2;
    for (size_t j=start + (!even_route && !reverse); j<end; j=j+2)
    {
        if ((type == LongRangeBell::Prep) || (type == LongRangeBell::Split))
        {
            id1 = std::nullopt; id2 = std::nullopt;
            if (j == start + (!even_route && !reverse))
                id1 = side2;
            if (j == end - 1 - (!even_route && reverse))
                id2 = side1;
            layer1.push_back({LocalInstruction::BellPrepare{id1, id2, route.cells[j].cell,route.cells[j+1].cell}});
        }

        else if ((type == LongRangeBell::Measure) || (type == LongRangeBell::Merge))
            layer2.push_back({LocalInstruction::BellMeasure{route.cells[j].cell,route.cells[j+1].cell}});
    }
    for (size_t j=start + 2 + (!even_route && !reverse); j<end; j=j+2)
    {
        if ((type == LongRangeBell::Prep) || (type == LongRangeBell::Split))
            layer2.push_back({LocalInstruction::BellMeasure{route.cells[j-1].cell, route.cells[j].cell}});

        else if ((type == LongRangeBell::Measure) || (type == LongRangeBell::Merge))
        {
            id1 = std::nullopt; id2 = std::nullopt;
            if (!reverse && (j == start + 2 + (!even_route && !reverse)))
                id1 = side2;
            else if (reverse && (j == end - 1 - !even_route))
                id2 = side1;
            layer1.push_back({LocalInstruction::BellPrepare{id1, id2, route.cells[j-1].cell, route.cells[j].cell}});
        }
    }

}

InstructionApplicationResult try_apply_instruction_direct_followup(
        DenseSlice& slice,
        LSInstruction& instruction,
        bool local_instructions,
        bool allow_twists,
        bool gen_op_ids,
        const Layout& layout,
        Router& router)
{
    if (const auto* s = std::get_if<SinglePatchMeasurement>(&instruction.operation))
    {
        auto maybe_target_patch = slice.get_patch_by_id(s->target);
        if(!maybe_target_patch)
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", s->target, " not on lattice")), {}};

        auto& target_patch = maybe_target_patch->get();
        target_patch.activity = PatchActivity::Measurement;
        
        return {nullptr, {}};
    }
    else if (const auto* p = std::get_if<SingleQubitOp>(&instruction.operation))
    {
        auto maybe_target_patch = slice.get_patch_by_id(p->target);
        if (!maybe_target_patch)
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", p->target, " not on lattice")), {}};
        auto& target_patch = maybe_target_patch->get();

        if (p->op==SingleQubitOp::Operator::S)
        {
            if (router.get_EDPC())
                throw std::runtime_error("EDPC is not available for non-local lattice surgery operations.");

            if (!merge_patches(slice, router, p->target, PauliOperator::X, p->target, PauliOperator::Z, gen_op_ids))
                return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Could not do S gate routing on ", p->target)),
                        {}};
            LSInstruction corrective_term{SingleQubitOp{p->target, SingleQubitOp::Operator::Z}};
            return {nullptr, {corrective_term}};
        }
        else
        {
            if (target_patch.is_active())
                return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", p->target, " is active")), {}};
            
            if (p->op == SingleQubitOp::Operator::H)
            {
                target_patch.activity = PatchActivity::Unitary;
                target_patch.boundaries.instant_rotate();
            }
                
            return {nullptr, {}};
        }
    }
    else if (auto* m = std::get_if<MultiPatchMeasurement>(&instruction.operation))
    {

        if (m->observable.size()!=2)
            throw std::logic_error(lstk::cat(instruction,"; Multi patch measurement only supports 2 patches currently. Got:\n", *m));
        auto pairs = m->observable.begin();
        const auto&[source_id, source_op] = *pairs++;
        const auto&[target_id, target_op] = *pairs;

        if (!slice.has_patch(source_id))
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", source_id, " not on lattice")), {}};
        
        if (!slice.has_patch(target_id))
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", target_id, " not on lattice")), {}};

        if (!local_instructions) 
        {
            if (router.get_EDPC())
                throw std::runtime_error("EDPC is not available for non-local lattice surgery operations.");
            if (!merge_patches(slice, router, source_id, source_op, target_id, target_op, gen_op_ids))
                return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Couldn't find room to route")), {}};
            
            return {nullptr, {}};
        }

        else
        {
            // We assume the ability to do ZZ, XX, and XZ local lattice surgery operations, but the appropriate operator edges must already be touching
            // TODO: insert patch rotations to align the appropriate operator edges if they are misaligned
            Boundary b1 = slice.get_boundary_between_or_fail(slice.get_cell_by_id(source_id).value(),slice.get_cell_by_id(target_id).value()).get();
            Boundary b2 = slice.get_boundary_between_or_fail(slice.get_cell_by_id(target_id).value(),slice.get_cell_by_id(source_id).value()).get();
            if (((b1.boundary_type == BoundaryType::Rough) && (source_op != PauliOperator::X)) ||
                ((b1.boundary_type == BoundaryType::Smooth) && (source_op != PauliOperator::Z)))
            {
                std::logic_error{lstk::cat(instruction,"; Boundary/operator type mismatch")};
            } 
            if (((b2.boundary_type == BoundaryType::Rough) && (target_op != PauliOperator::X)) ||
                ((b2.boundary_type == BoundaryType::Smooth) && (target_op != PauliOperator::Z)))
            {
                std::logic_error{lstk::cat(instruction,"; Boundary/operator type mismatch")};
            } 

            // Apply TwoPatchMeasure local instruction
            LocalInstruction::LocalLSInstruction local_instruction = {LocalInstruction::TwoPatchMeasure{slice.get_cell_by_id(source_id).value(), slice.get_cell_by_id(target_id).value()}};
            m->local_instruction = std::move(local_instruction);
            InstructionApplicationResult r = try_apply_local_instruction(slice, m->local_instruction.value(), layout, router);
            if (r.maybe_error && r.followup_instructions.empty())
                return InstructionApplicationResult{std::move(r.maybe_error), {}};
            if (!r.followup_instructions.empty())
                return InstructionApplicationResult{std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Followup local instructions not implemented")), {}};
            return {nullptr, {}};

            // TODO: Implement local instruction compilation/decomposition for more general MultiPatchMeasurements
        }


    }
    else if (const auto* init = std::get_if<PatchInit>(&instruction.operation))
    {
        auto location= init->place_next_to ?
                  place_ancilla_next_to(slice, init->place_next_to->target, init->place_next_to->op)
                : find_free_ancilla_location(layout, slice);
        if (!location) return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Could not allocate ancilla")), {}};

        slice.patch_at(*location);
        slice.place_single_cell_sparse_patch(LayoutHelpers::basic_square_patch(*location, std::nullopt, "Init"), false);
        slice.patch_at(*location)->id = init->target;

        return {nullptr, {}};
    }
    else if (auto* bell_init = std::get_if<BellPairInit>(&instruction.operation))
    {
        if (!local_instructions)
        {
            throw std::runtime_error("BellPairInit not implemented unless local compilation flag is given.");
        }

        if (!slice.has_patch(bell_init->loc1.target))
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", bell_init->loc1.target, " not on lattice")), {}};
        if (!slice.has_patch(bell_init->loc2.target)) 
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", bell_init->loc2.target, " not on lattice")), {}};

        else 
        {
            if (!bell_init->counter.has_value())
            {
                auto routing_region = router.find_routing_ancilla(slice, bell_init->loc1.target, bell_init->loc1.op, bell_init->loc2.target, bell_init->loc2.op);
                if(!routing_region)
                {
                    return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; No valid route found for Bell pair creation")), {}};
                }
                else if (routing_region->cells.size() < 2) 
                {
                    return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Shortest route cannot be used for Bell pair creation")), {}};
                }

                std::vector<LocalInstruction::LocalLSInstruction> local_instructions;
                std::vector<LocalInstruction::LocalLSInstruction> layer1;
                std::vector<LocalInstruction::LocalLSInstruction> layer2;
                layer1.reserve(routing_region->cells.size()); layer2.reserve(routing_region->cells.size());
                local_instructions.reserve(2*routing_region->cells.size());

                compile_long_range_Bell(LongRangeBell::Prep, layer1, layer2, routing_region.value(), 0, routing_region->cells.size()-1, false, bell_init->side1, bell_init->side2);
                local_instructions.insert(local_instructions.end(), layer1.begin(), layer1.end());
                local_instructions.insert(local_instructions.end(), layer2.begin(), layer2.end());

                bell_init->local_instructions = std::move(local_instructions);
                bell_init->counter = std::pair<unsigned int, unsigned int>(0, 0);
            }

            bell_init->counter->first = bell_init->counter->second;
            for (unsigned int i = bell_init->counter->first; i < bell_init->local_instructions.value().size(); i++)
            {
                InstructionApplicationResult r = try_apply_local_instruction(slice, bell_init->local_instructions.value()[i], layout, router);
                if (r.maybe_error && r.followup_instructions.empty())
                    return InstructionApplicationResult{nullptr, {instruction}};
                if (!r.followup_instructions.empty())
                    return InstructionApplicationResult{std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Followup local instructions not implemented")), {}};

                bell_init->counter->second++;
            }
            return {nullptr, {}};
        }
    }
    else if (auto* bell_cnot = std::get_if<BellBasedCNOT>(&instruction.operation))
    {
        if (!local_instructions)
        {
            throw std::runtime_error("BellBasedCNOT not implemented unless local compilation flag is given.");
        }
        
        if (!slice.has_patch(bell_cnot->control))
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", bell_cnot->control, " not on lattice")), {}};
        if (!slice.has_patch(bell_cnot->target)) 
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", bell_cnot->target, " not on lattice")), {}};
        
        auto control_patch = slice.get_patch_by_id(bell_cnot->control);
        auto target_patch = slice.get_patch_by_id(bell_cnot->target);
        
        if (control_patch.value().get().is_active())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", bell_cnot->control, " is active")), {}};
        if (target_patch.value().get().is_active())
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", bell_cnot->target, " is active")), {}};
        
        else 
        {
            // If we are using EDPC, we defer local compilation until the whole EDP set has been found within a wave. 
            if (router.get_EDPC())
            {

                // If this is the first pass through, then we find a non-conflicting path and mark it
                if (!bell_cnot->route.has_value())
                {
                    auto routing_region = router.find_routing_ancilla(slice, bell_cnot->control, PauliOperator::Z, bell_cnot->target, PauliOperator::X);
                    if(!routing_region) 
                    {
                        return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; No valid route found for Bell pair creation")), {}};
                    }
                    else if (routing_region->cells.size() < 2) 
                    {
                        throw std::runtime_error("BellBasedCNOT requires shortest route to be at least two tiles.");
                    }

                    // Add control and target to the routing region (helps with setting boundary labels)
                    routing_region->cells.push_back(SingleCellOccupiedByPatch{
                                        {.top=   {BoundaryType::None, false},
                                        .bottom={BoundaryType::None, false},
                                        .left=  {BoundaryType::None, false},
                                        .right= {BoundaryType::None, false}},
                                        slice.get_cell_by_id(bell_cnot->control).value()});

                    routing_region->cells.insert(routing_region->cells.begin(), SingleCellOccupiedByPatch{
                                        {.top=   {BoundaryType::None, false},
                                        .bottom={BoundaryType::None, false},
                                        .left=  {BoundaryType::None, false},
                                        .right= {BoundaryType::None, false}},
                                        slice.get_cell_by_id(bell_cnot->target).value()});

                    mark_routing_region(slice, *routing_region, PatchActivity::EDPC);      

                    // Store routing region within the instruction (importantly, boundaries of cells in this routing region object have not been re-labeled!)
                    bell_cnot->route = std::move(routing_region);

                    // We return the instruction back to the scheduler and perform local instruction decomposition and compilation later
                    return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; EDPC requires whole EDP set to be found before local compilation can occur")), {instruction}};
                }

                else
                {

                    // On the second pass through, we use the edge labels to construct local compilation
                    if (!bell_cnot->counter_pairs.has_value())
                    {
                        // Determine which path type we're starting with
                        auto current_type = slice.get_boundary_between(bell_cnot->route->cells[0].cell, bell_cnot->route->cells[1].cell).value().get().boundary_type;
  
                        // Iterate over route to find route endpoints and associate them with labels
                        std::vector<std::pair<size_t, BoundaryType>> route_endpoints = {std::make_pair(0, current_type)};
                        for (size_t i=1; i<bell_cnot->route->cells.size()-1; i++)
                        {  
                            auto new_type = slice.get_boundary_between(bell_cnot->route->cells[i].cell, bell_cnot->route->cells[i+1].cell).value().get().boundary_type;
                            if (new_type != current_type)
                            {
                                current_type = new_type;
                                route_endpoints.push_back(std::make_pair(i, current_type));
                            }
                        }
                        route_endpoints.push_back(std::make_pair(bell_cnot->route->cells.size()-1, current_type));

                        // Loop over end-points to decompose operation along path into local operations organized into two phases of two layers each                        
                        std::vector<LocalInstruction::LocalLSInstruction> phase1_layer1;
                        std::vector<LocalInstruction::LocalLSInstruction> phase1_layer2;
                        std::vector<LocalInstruction::LocalLSInstruction> phase2_layer1;
                        std::vector<LocalInstruction::LocalLSInstruction> phase2_layer2;

                        phase1_layer1.reserve(bell_cnot->route->cells.size());
                        phase1_layer2.reserve(bell_cnot->route->cells.size());
                        phase2_layer1.reserve(bell_cnot->route->cells.size());
                        phase2_layer2.reserve(bell_cnot->route->cells.size());

                        for (size_t i=0; i<route_endpoints.size()-1; i++) 
                        {

                            // If there is only one segment, we apply in the same way as without EDPC
                            if ((route_endpoints.size() == 2) && (current_type == BoundaryType::Reserved_Label1))
                            {
                                if ((bell_cnot->route->cells.size()%2) == 0)
                                {
                                    compile_long_range_Bell(LongRangeBell::Merge, phase1_layer1, phase1_layer2, bell_cnot->route.value(), bell_cnot->route->cells.size()-2, bell_cnot->route->cells.size()-1, false);
                                    compile_long_range_Bell(LongRangeBell::Merge, phase1_layer1, phase1_layer2, bell_cnot->route.value(), 0, 1, true);
                                    compile_long_range_Bell(LongRangeBell::Prep, phase1_layer1, phase1_layer2, bell_cnot->route.value(), 1, bell_cnot->route->cells.size()-2, true, bell_cnot->side1, bell_cnot->side2);
                                }
                                else
                                {
                                    compile_long_range_Bell(LongRangeBell::Split, phase1_layer1, phase1_layer2, bell_cnot->route.value(), bell_cnot->route->cells.size()-2, bell_cnot->route->cells.size()-1, false);
                                    compile_long_range_Bell(LongRangeBell::Merge, phase1_layer1, phase1_layer2, bell_cnot->route.value(), 0, bell_cnot->route->cells.size()-2, true,  bell_cnot->side1, bell_cnot->side2);
                                }
                            }

                            else if ((route_endpoints.size() == 2) && (current_type == BoundaryType::Reserved_Label2))
                            {
                                if ((bell_cnot->route->cells.size()%2) == 0)
                                {
                                    compile_long_range_Bell(LongRangeBell::Merge, phase2_layer1, phase2_layer2, bell_cnot->route.value(), bell_cnot->route->cells.size()-2, bell_cnot->route->cells.size()-1, false);
                                    compile_long_range_Bell(LongRangeBell::Merge, phase2_layer1, phase2_layer2, bell_cnot->route.value(), 0, 1, true);
                                    compile_long_range_Bell(LongRangeBell::Prep, phase2_layer1, phase2_layer2, bell_cnot->route.value(), 1, bell_cnot->route->cells.size()-2, true, bell_cnot->side1, bell_cnot->side2);
                                }
                                else
                                {
                                    compile_long_range_Bell(LongRangeBell::Split, phase2_layer1, phase2_layer2, bell_cnot->route.value(), bell_cnot->route->cells.size()-2, bell_cnot->route->cells.size()-1, false);
                                    compile_long_range_Bell(LongRangeBell::Merge, phase2_layer1, phase2_layer2, bell_cnot->route.value(), 0, bell_cnot->route->cells.size()-2, true,  bell_cnot->side1, bell_cnot->side2);
                                }
                            }

                            // Otherwise, check segment type
                            else if (route_endpoints[i].second == BoundaryType::Reserved_Label1)
                            {
                                // Phase 1 operation involving control/target are Bell-based splits. Otherwise, it is a long-range Bell preparation
                                if (i==0)
                                {
                                    // Split target
                                    compile_long_range_Bell(LongRangeBell::Split, phase1_layer1, phase1_layer2, bell_cnot->route.value(), route_endpoints[i].first, route_endpoints[i+1].first, true,  std::nullopt, bell_cnot->side2);
                                }

                                else if (i==route_endpoints.size()-2)
                                {
                                    // Split control 
                                    compile_long_range_Bell(LongRangeBell::Split, phase1_layer1, phase1_layer2, bell_cnot->route.value(), route_endpoints[i].first, route_endpoints[i+1].first, false, std::nullopt, bell_cnot->side1); 

                                }

                                else 
                                {
                                    // Prepare Bell state
                                    compile_long_range_Bell(LongRangeBell::Prep, phase1_layer1, phase1_layer2, bell_cnot->route.value(), route_endpoints[i].first, route_endpoints[i+1].first); 
                                }
                            }

                            else if (route_endpoints[i].second == BoundaryType::Reserved_Label2)
                            {
                                // Phase 2 operation involving control/target are Bell-based merges. Otherwise, it is a long-range Bell measurement
                                if (i==0)
                                {
                                    // Merge target
                                    compile_long_range_Bell(LongRangeBell::Merge, phase2_layer1, phase2_layer2, bell_cnot->route.value(), route_endpoints[i].first, route_endpoints[i+1].first, true,  std::nullopt, std::nullopt);
                                }

                                else if (i==route_endpoints.size()-2)
                                {
                                    // Merge control 
                                    compile_long_range_Bell(LongRangeBell::Merge, phase2_layer1, phase2_layer2, bell_cnot->route.value(), route_endpoints[i].first, route_endpoints[i+1].first, false, std::nullopt, std::nullopt); 

                                }

                                else 
                                {
                                    // Perform Bell measurement
                                    compile_long_range_Bell(LongRangeBell::Measure, phase2_layer1, phase2_layer2, bell_cnot->route.value(), route_endpoints[i].first, route_endpoints[i+1].first); 
                                }
                            }

                            else
                            {
                                throw std::runtime_error("Invalid BoundaryType in EDPC compilation.");
                            }

                        }

                        // std::cerr << "PHASE 1 LAYER 1: " << std::endl;
                        // for (const auto& instr : phase1_layer1)
                        // {
                        //     std::cerr << instr << "; ";
                        // }
                        // std::cerr << std::endl;

                        // std::cerr << "PHASE 1 LAYER 2: " << std::endl;
                        // for (const auto& instr : phase1_layer2)
                        // {
                        //     std::cerr << instr << "; ";
                        // }
                        // std::cerr << std::endl;

                        // std::cerr << "PHASE 2 LAYER 1: " << std::endl;
                        // for (const auto& instr : phase2_layer1)
                        // {
                        //     std::cerr << instr << "; ";
                        // }
                        // std::cerr << std::endl;

                        // std::cerr << "PHASE 2 LAYER 2: " << std::endl;
                        // for (const auto& instr : phase2_layer2)
                        // {
                        //     std::cerr << instr << "; ";
                        // }
                        // std::cerr << std::endl;

                        // Add local instruction sets to LLI object and initialize tracking variables
                        bell_cnot->local_instruction_sets = {std::move(phase1_layer1), std::move(phase1_layer2), std::move(phase2_layer1), std::move(phase2_layer2)};
                        bell_cnot->counter_pairs = {std::pair<unsigned int, unsigned int>(0, 0), std::pair<unsigned int, unsigned int>(0, 0), 
                            std::pair<unsigned int, unsigned int>(0, 0), std::pair<unsigned int, unsigned int>(0, 0)};
                        bell_cnot->current_phase = 0;

                        // We (once again) return the instruction back to the scheduler and apply local instructions later (once the decomposition and scheduling has occurred for all EDP)
                        return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; EDPC requires all CNOTs to be decomposed into local instructions before slicing can occur.")), {}};
                    }

                    // Loop over instructions in the current phase
                    size_t phase = bell_cnot->current_phase.value();
                    if (phase != 0)
                    {
                        bell_cnot->counter_pairs.value()[phase-1].first = bell_cnot->counter_pairs.value()[phase-1].second; 
                    }
                    bell_cnot->counter_pairs.value()[phase].first = bell_cnot->counter_pairs.value()[phase].second;
                    for (unsigned int i = bell_cnot->counter_pairs.value()[phase].first; i < bell_cnot->local_instruction_sets.value()[phase].size(); i++)
                    {
                        InstructionApplicationResult r = try_apply_local_instruction(slice, bell_cnot->local_instruction_sets.value()[phase][i], layout, router);
                        if (r.maybe_error && r.followup_instructions.empty())
                        {
                            return {nullptr, {instruction}};
                        }
                        if (!r.followup_instructions.empty())
                            throw std::runtime_error(lstk::cat(instruction,"; Followup local instructions not implemented"));
                        
                        bell_cnot->counter_pairs.value()[phase].second++;
                    }

                    // If this is the final phase, the instruction has been fully applied 
                    if ((phase == bell_cnot->local_instruction_sets.value().size() - 1) ||
                        ((phase == 1) && (bell_cnot->local_instruction_sets.value()[2].size() == 0)))
                    {
                        return {nullptr, {}};
                    }

                    // Otherwise, the phase is incremented and the instruction returns itself as a follow-up 
                    else
                    {
                        bell_cnot->current_phase.value()++;
                        return {nullptr, {instruction}};
                    }
                    
                }
            }

            else {

                if (!bell_cnot->counter_pairs.has_value())
                {
                    auto routing_region = router.find_routing_ancilla(slice, bell_cnot->control, PauliOperator::Z, bell_cnot->target, PauliOperator::X);
                    if(!routing_region) 
                    {
                        return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; No valid route found for Bell pair creation")), {}};
                    }
                    else if (routing_region->cells.size() < 2) 
                    {
                        throw std::runtime_error("BellBasedCNOT requires shortest route to be at least two tiles.");
                    }

                    // Add control and target to the routing region (helps with setting boundary labels)
                    routing_region->cells.push_back(SingleCellOccupiedByPatch{
                                        {.top=   {BoundaryType::None, false},
                                        .bottom={BoundaryType::None, false},
                                        .left=  {BoundaryType::None, false},
                                        .right= {BoundaryType::None, false}},
                                        slice.get_cell_by_id(bell_cnot->control).value()});

                    routing_region->cells.insert(routing_region->cells.begin(), SingleCellOccupiedByPatch{
                                        {.top=   {BoundaryType::None, false},
                                        .bottom={BoundaryType::None, false},
                                        .left=  {BoundaryType::None, false},
                                        .right= {BoundaryType::None, false}},
                                        slice.get_cell_by_id(bell_cnot->target).value()});

                    // Allocate vectors to hold local instructions
                    std::vector<LocalInstruction::LocalLSInstruction> local_instructions;
                    std::vector<LocalInstruction::LocalLSInstruction> layer1;
                    std::vector<LocalInstruction::LocalLSInstruction> layer2;
                    layer1.reserve(routing_region->cells.size()); layer2.reserve(routing_region->cells.size());
                    local_instructions.reserve(2*routing_region->cells.size());

                    // Add local instructions (two time-steps requires different compilations)
                    if ((routing_region->cells.size()%2) == 0)
                    {
                        compile_long_range_Bell(LongRangeBell::Merge, layer1, layer2, routing_region.value(), routing_region->cells.size()-2, routing_region->cells.size()-1, false);
                        compile_long_range_Bell(LongRangeBell::Merge, layer1, layer2, routing_region.value(), 0, 1, true);
                        compile_long_range_Bell(LongRangeBell::Prep, layer1, layer2, routing_region.value(), 1, routing_region->cells.size()-2, true, bell_cnot->side1, bell_cnot->side2);
                    }
                    else
                    {
                        compile_long_range_Bell(LongRangeBell::Split, layer1, layer2, routing_region.value(), routing_region->cells.size()-2, routing_region->cells.size()-1, false);
                        compile_long_range_Bell(LongRangeBell::Merge, layer1, layer2, routing_region.value(), 0, routing_region->cells.size()-2, true,  bell_cnot->side1, bell_cnot->side2);
                    }
                    local_instructions.insert(local_instructions.end(), layer1.begin(), layer1.end());
                    local_instructions.insert(local_instructions.end(), layer2.begin(), layer2.end());

                    bell_cnot->local_instruction_sets = {std::move(local_instructions)};
                    bell_cnot->counter_pairs = {std::pair<unsigned int, unsigned int>(0, 0)};
                }

                bell_cnot->counter_pairs.value()[0].first = bell_cnot->counter_pairs.value()[0].second;
                
                for (unsigned int i = bell_cnot->counter_pairs.value()[0].first; i < bell_cnot->local_instruction_sets.value()[0].size(); i++)
                {
                    InstructionApplicationResult r = try_apply_local_instruction(slice, bell_cnot->local_instruction_sets.value()[0][i], layout, router);
                    if (r.maybe_error && r.followup_instructions.empty())
                        return InstructionApplicationResult{nullptr, {instruction}};
                    if (!r.followup_instructions.empty())
                        return InstructionApplicationResult{std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Followup local instructions not implemented")), {}};

                    bell_cnot->counter_pairs.value()[0].second++;
                }
                return {nullptr, {}};
            }
        }
    }
    else if (const auto* rotation = std::get_if<RotateSingleCellPatch>(&instruction.operation))
    {
        if (!slice.has_patch(rotation->target))
            return {std::make_unique<std::runtime_error>(lstk::cat(instruction,"; Patch ", rotation->target, " not on lattice")), {}};
        const Cell target_cell = *slice.get_cell_by_id(rotation->target);

        std::optional<Cell> free_neighbour;

        for (auto neighbour_cell: slice.get_neigbours_within_slice(target_cell))
            if (slice.is_cell_free(neighbour_cell))
                free_neighbour = neighbour_cell;

        if (!free_neighbour)
            return {std::make_unique<std::runtime_error>(lstk::cat(
                    instruction,"; Cannot rotate patch ", rotation->target, ": has no free neighbour")), {}};

        if (slice.patch_at(target_cell)->activity == PatchActivity::Unitary)
        {
            slice.patch_at(target_cell)->activity = PatchActivity::None;
        }

        BusyRegion rotation_instruction{single_patch_rotation_a_la_litinski(slice.patch_at(target_cell)->to_sparse_patch(target_cell), *free_neighbour)};

        slice.patch_at(target_cell) = std::nullopt;
        mark_routing_region(slice, rotation_instruction.region, PatchActivity::Rotation);

        rotation_instruction.steps_to_clear--;
        return {nullptr, {{rotation_instruction}}};
    }
    else if (auto* mr = std::get_if<MagicStateRequest>(&instruction.operation))
    {
        const auto& d_times = layout.distillation_times();
        if (!d_times.size()) throw std::logic_error("No distillation times");

        if (slice.magic_states.size()>0)
        {
            std::optional<Cell> min_cell;
            double min_dist = std::numeric_limits<double>::max();
            double dist;
            for (const Cell& cell : slice.magic_states)
            {
                dist = abs(cell.col - slice.get_cell_by_id(mr->near_patch).value().col) + abs(cell.row - slice.get_cell_by_id(mr->near_patch).value().row);
                if (dist < min_dist)
                {
                    min_dist = dist;
                    min_cell = cell;
                }
            }

            assert(min_cell.has_value());
            auto& newly_bound_magic_state = slice.patch_at(min_cell.value()).value();
            newly_bound_magic_state.id = mr->target;
            newly_bound_magic_state.type = PatchType::Qubit;
            newly_bound_magic_state.activity = PatchActivity::None;
            slice.magic_states.erase(min_cell.value());
            return {nullptr, {}};
        }
        else 
        {
            return {std::make_unique<std::runtime_error>(
                    lstk::cat(instruction,";Could not get magic state")), {}};           
        }
    }
    else if (auto* yr = std::get_if<YStateRequest>(&instruction.operation))
    {
        // Unbind Y state patch if already bound
        if (slice.get_patch_by_id(yr->target))
        {
            slice.get_patch_by_id(yr->target)->get().id = std::nullopt;
            slice.predistilled_ystates_available++;
            return{nullptr, {}};
        }

        // Otherwise, bind a new or existing Y state
        else 
        {

            // Use pre-distilled y states if there are any 

            // We don't allow the pre-distilled Y states to be used if both allow_twists and local_instructions are selected, 
            // since in that case we use the teleported S gate injection stream which relies on ZZ measurement rather than CNOT gates
            if (!(local_instructions && allow_twists) && (slice.predistilled_ystates_available != 0)) {

                // Get minimum (L1) distance unbound Y state patch
                std::optional<Cell> min_cell;
                double min_dist = std::numeric_limits<double>::max();
                double dist;
                for (const Cell& cell : layout.predistilled_y_states())
                {
                    if (!slice.patch_at(cell)->id.has_value() && !slice.patch_at(cell)->is_active())
                    {
                        dist = abs(cell.col - slice.get_cell_by_id(yr->near_patch).value().col) + abs(cell.row - slice.get_cell_by_id(yr->near_patch).value().row);
                        if (dist < min_dist)
                        {
                            min_dist = dist;
                            min_cell = cell;
                        }
                    }
                }

                if (min_cell)
                {
                    auto& patch = slice.patch_at(min_cell.value());
                    if (patch)
                    {
                        slice.patch_at(min_cell.value()).value().id = yr->target;
                        slice.predistilled_ystates_available--;
                        return {nullptr, {}};
                    }
                }
            }

            // Otherwise, need to prepare a Y state
            //     Ideally, it would be at the free cell that has the shortest route to the target qubit by Z edges
            //     For now, we just place it directly next-door if possible (suitable for local compilation)
            else {

                if (allow_twists)
                {
                    
                    // Find all free neighboring cells
                    Cell target_cell = slice.get_cell_by_id(yr->near_patch).value();
                    std::optional<Cell> z_neighbour;
                    for (auto neighbour_cell: slice.get_neigbours_within_slice(target_cell))
                        if (slice.is_cell_free(neighbour_cell))
                        {
                            // Check if Z boundary of target patch is adjacent to this neighbor
                            Boundary b = slice.get_boundary_between(target_cell, neighbour_cell)->get();
                            if (b.boundary_type == BoundaryType::Smooth)
                                z_neighbour = neighbour_cell;
                        }
                    if (!z_neighbour)
                        return {std::make_unique<std::runtime_error>(lstk::cat(
                                instruction,"; Cannot prepare Y state: ", yr->near_patch, " has no neighbor to its Z edge")), {}};
                    else
                    {
                        LocalInstruction::LocalLSInstruction local_instruction = {LocalInstruction::PrepareY{yr->target, z_neighbour.value()}};
                        yr->local_instruction = std::move(local_instruction);
                        InstructionApplicationResult r = try_apply_local_instruction(slice, local_instruction, layout, router);                       
                        return {nullptr, {}};
                    }

                }

            }

            return {std::make_unique<std::runtime_error>(
                    std::string{"Could not get Y state"}), {}};           
        }

    }
    else if (auto* busy_region = std::get_if<BusyRegion>(&instruction.operation))
    {
        if(busy_region->steps_to_clear <= 0)
        {
            clear_routing_region(slice, busy_region->region);

            for (const SparsePatch& patch : busy_region->state_after_clearing) {
                bool could_not_find_space_for_patch = false;
                patch.visit_individual_cells(
                        [&](const SingleCellOccupiedByPatch& occupied_cell){
                            if(!slice.is_cell_free(occupied_cell.cell))
                                could_not_find_space_for_patch = true;
                        });

                if(could_not_find_space_for_patch)
                    return {std::make_unique<std::runtime_error>(lstk::cat(
                            instruction,"; Could not find space to place patch after BusyRegion clears")),{}};
                slice.place_sparse_patch(patch, false);
            }

            return {nullptr,{}};
        }
        else
        {
            for(const auto& occupied_cell: busy_region->region.cells)
            {
                auto& patch = slice.patch_at(occupied_cell.cell);
                assert(patch && "Busy region route is uninitialized");
            }
            
            return {nullptr,{{BusyRegion{
                    busy_region->region,
                    busy_region->steps_to_clear-1,
                    busy_region->state_after_clearing}}}};
        }

    }
    else if (std::get_if<PatchReset>(&instruction.operation))
        return {nullptr, {}};
    
    LSTK_UNREACHABLE;
}

namespace {

// Pull-based stream version of the (default) streamed pipeline. The original push loop emitted
// the current slice (slice_visitor) and then advanced it; here next() returns the slice and
// defers the post-emit work to the following call, so the returned reference stays valid for
// the duration of the consumer's iteration. Two pieces of state carry across that boundary:
//   - needs_advance_: a followup slice was emitted; advance it before producing the next one.
//   - pending_error_: an unroutable instruction's slice was emitted; the original code runs the
//     "throw / note-no-progress / requeue / advance" tail *after* that emit, so we replay it at
//     the top of the next call (preserving that the failing slice is emitted before any throw).
class StreamedSliceStream : public DenseSliceStream
{
public:
    StreamedSliceStream(
            std::unique_ptr<LSInstructionStream> instruction_stream,
            const Layout& layout,
            std::unique_ptr<Router> router,
            LSInstructionVisitor instruction_visitor,
            const DenseSlicingOptions& options)
        : instruction_stream_(std::move(instruction_stream)),
          options_(options),
          layout_(layout),
          router_(std::move(router)),
          instruction_visitor_(std::move(instruction_visitor)),
          slice_(layout, instruction_stream_->core_qubits()),
          // A permanently stuck instruction (e.g. one the layout cannot route) is retried at the
          // front every slice; the guard bails with a diagnostic instead of looping forever.
          no_progress_guard_(options.max_no_progress_slices)
    {}

    const DenseSlice* next() override
    {
        // Replay the deferred post-emit work from the previous slice.
        if (pending_error_)
        {
            LSInstruction instruction = std::move(pending_error_->instruction);
            std::string cause = std::move(pending_error_->cause);
            pending_error_.reset();

            if (instruction.wait_at_most_for == 0)
                throw std::runtime_error{cause};

            no_progress_guard_.note_no_progress(&instruction, cause);

            instruction.wait_at_most_for--;
            future_instructions_.push_front(std::move(instruction));
            advance_slice(slice_, layout_);
        }
        else if (needs_advance_)
        {
            advance_slice(slice_, layout_);
            needs_advance_ = false;
        }

        while (instruction_stream_->has_next_instruction() || !future_instructions_.empty())
        {
            LSInstruction instruction = [&]()
            {
                if (!future_instructions_.empty())
                    return lstk::deque_pop(future_instructions_);
                result_.ls_instructions_count_++;
                return instruction_stream_->get_next_instruction();
            }();

            auto application_result = try_apply_instruction_direct_followup(slice_, instruction, options_.local_instructions, options_.allow_twists, options_.gen_op_ids, layout_, *router_);
            if (!application_result.maybe_error)
            {
                instruction_visitor_(instruction);
                no_progress_guard_.note_progress();
            }

            if (!application_result.followup_instructions.empty())
            {
                result_.slice_count_++;
                for (auto&& i: application_result.followup_instructions)
                    future_instructions_.push_back(i);

                needs_advance_ = true;
                return &slice_;
            }
            else if (application_result.maybe_error)
            {
                pending_error_ = PendingError{std::move(instruction), application_result.maybe_error->what()};
                return &slice_;
            }
        }

        if (!emitted_final_)
        {
            emitted_final_ = true;
            return &slice_;
        }
        return nullptr;
    }

private:
    struct PendingError
    {
        LSInstruction instruction;
        std::string cause;
    };

    std::unique_ptr<LSInstructionStream> instruction_stream_;
    DenseSlicingOptions options_;
    const Layout& layout_;
    std::unique_ptr<Router> router_;
    LSInstructionVisitor instruction_visitor_;
    DenseSlice slice_;
    NoProgressGuard no_progress_guard_;
    std::deque<LSInstruction> future_instructions_;
    bool needs_advance_ = false;
    bool emitted_final_ = false;
    std::optional<PendingError> pending_error_;
};

} // namespace


namespace {

// Pull-based stream version of the (deprecated) dag pipeline. Every iteration of the original
// loop applied a round of proximate + non-proximate instructions and emitted exactly one slice,
// so next() does one such round per call and defers the trailing advance + slice count.
class DagSliceStream : public DenseSliceStream
{
public:
    DagSliceStream(
            dag::DependencyDag<LSInstruction>&& dag,
            const tsl::ordered_set<PatchId>& core_qubits,
            const Layout& layout,
            std::unique_ptr<Router> router,
            LSInstructionVisitor instruction_visitor,
            const DenseSlicingOptions& options)
        : dag_(std::move(dag)),
          options_(options),
          layout_(layout),
          router_(std::move(router)),
          instruction_visitor_(std::move(instruction_visitor)),
          slice_(layout, core_qubits)
    {}

    const DenseSlice* next() override
    {
        if (needs_advance_)
        {
            advance_slice(slice_, layout_);
            result_.slice_count_++;
            needs_advance_ = false;
        }

        if (dag_.empty())
            return nullptr;

        // Apply all proximate instructions
        auto proximate_instructions = dag_.proximate_instructions();
        for (dag::label_t instruction_label: proximate_instructions)
        {
            LSInstruction& instruction = dag_.at(instruction_label);
            auto application_result = try_apply_instruction_direct_followup(slice_, instruction, options_.local_instructions, options_.allow_twists, options_.gen_op_ids, layout_, *router_);
            if (application_result.maybe_error)
                throw std::runtime_error{lstk::cat(
                    "Could not apply proximate instruction:\n",
                    instruction,"\n",
                    "Caused by:\n",
                    application_result.maybe_error->what())};
            else
            {
                result_.ls_instructions_count_++;
                instruction_visitor_(instruction);
            }
            handle_followup_instructions(instruction_label, std::move(application_result.followup_instructions));

        }

        // Now apply all non-proximate instructions, where possible
        auto non_proximate_instructions = dag_.applicable_instructions();
        for (dag::label_t instruction_label: non_proximate_instructions)
        {
            LSInstruction& instruction = dag_.at(instruction_label);
            auto application_result = try_apply_instruction_direct_followup(slice_, instruction, options_.local_instructions, options_.allow_twists, options_.gen_op_ids, layout_, *router_);
            if (application_result.maybe_error)
            {
                increment_attempts(instruction_label);
                if (attempts_per_instruction_[instruction_label] > MAX_INSTRUCTION_APPLICATION_RETRIES_DAG_PIPELINE)
                {
                    throw std::runtime_error{lstk::cat(
                        "Could not apply non-proximate instruction after ",
                        MAX_INSTRUCTION_APPLICATION_RETRIES_DAG_PIPELINE," retries:\n",
                        instruction,"\n",
                        "Caused by:\n",
                        application_result.maybe_error->what())};
                }
            }
            else
            {
                result_.ls_instructions_count_++;
                instruction_visitor_(instruction);
                handle_followup_instructions(instruction_label, std::move(application_result.followup_instructions));
            }
        }

        needs_advance_ = true;
        return &slice_;
    }

private:
    void increment_attempts(dag::label_t label)
    {
        if (!attempts_per_instruction_.contains(label))
            attempts_per_instruction_[label] = 0;
        attempts_per_instruction_[label]++;
    }

    void handle_followup_instructions(dag::label_t instruction_label, std::vector<LSInstruction>&& followup_instructions)
    {
        if (!followup_instructions.empty())
        {
            dag::label_t new_head = dag_.expand(instruction_label, std::move(followup_instructions), true);
            dag_.make_proximate(new_head);
        }
        else
            dag_.pop_head(instruction_label);
    }

    dag::DependencyDag<LSInstruction> dag_;
    DenseSlicingOptions options_;
    const Layout& layout_;
    std::unique_ptr<Router> router_;
    LSInstructionVisitor instruction_visitor_;
    DenseSlice slice_;
    std::unordered_map<dag::label_t, size_t> attempts_per_instruction_;
    bool needs_advance_ = false;
};

} // namespace

namespace {

// Pull-based stream version of the wave / EDPC pipeline. As in the dag pipeline each call
// schedules one wave and emits one slice, deferring the trailing advance + slice count. The
// WaveScheduler drains the instruction stream at construction, so the stream need not outlive it.
class WaveSliceStream : public DenseSliceStream
{
public:
    WaveSliceStream(
            std::unique_ptr<LSInstructionStream> instruction_stream,
            const Layout& layout,
            std::unique_ptr<Router> router,
            LSInstructionVisitor instruction_visitor,
            const DenseSlicingOptions& options)
        : layout_(layout),
          instruction_visitor_(std::move(instruction_visitor)),
          slice_(layout, instruction_stream->core_qubits()),
          scheduler_(std::move(*instruction_stream), layout, std::move(router), options),
          no_progress_guard_(options.max_no_progress_slices)
    {}

    const DenseSlice* next() override
    {
        if (needs_advance_)
        {
            advance_slice(slice_, layout_);
            result_.slice_count_++;
            needs_advance_ = false;
        }

        if (scheduler_.done())
            return nullptr;

        WaveStats wave_stats = scheduler_.schedule_wave(slice_, instruction_visitor_, result_);
        if (wave_stats.applied_wave_size > 0)
            no_progress_guard_.note_progress();
        else if (wave_stats.blocked_instruction)
            no_progress_guard_.note_no_progress(&*wave_stats.blocked_instruction, wave_stats.blocked_cause);
        else
            no_progress_guard_.note_no_progress();

        needs_advance_ = true;
        return &slice_;
    }

private:
    const Layout& layout_;
    LSInstructionVisitor instruction_visitor_;
    DenseSlice slice_;
    WaveScheduler scheduler_;
    NoProgressGuard no_progress_guard_;
    bool needs_advance_ = false;
};


// Decorator that aborts slicing once a wall-clock budget is exceeded. The check runs after each
// slice is produced but before it reaches the consumer, matching the original visitor wrapper.
class TimeoutSliceStream : public DenseSliceStream
{
public:
    TimeoutSliceStream(
            std::unique_ptr<DenseSliceStream> inner,
            std::chrono::seconds timeout)
        : inner_(std::move(inner)), timeout_(timeout), start_(lstk::now())
    {}

    const DenseSlice* next() override
    {
        const DenseSlice* slice = inner_->next();
        if (slice && lstk::since(start_) > timeout_)
        {
            const DensePatchComputationResult& res = inner_->result();
            throw std::runtime_error{
                std::string{"Out of time after "}+std::to_string(timeout_.count())+std::string{"s. "}
                +std::string{"Consumed "}+std::to_string(res.ls_instructions_count_)+std::string{" Instructions. "}
                +std::string{"Generated "}+std::to_string(res.slice_count_)+std::string{"Slices."}};
        }
        return slice;
    }

    const DensePatchComputationResult& result() const override { return inner_->result(); }

private:
    std::unique_ptr<DenseSliceStream> inner_;
    std::chrono::seconds timeout_;
    std::chrono::steady_clock::time_point start_;
};

} // namespace


std::unique_ptr<DenseSliceStream> run_through_dense_slices(
        std::unique_ptr<LSInstructionStream> instruction_stream,
        const Layout& layout,
        std::unique_ptr<Router> router,
        LSInstructionVisitor instruction_visitor,
        const DenseSlicingOptions& options)
{
    std::unique_ptr<DenseSliceStream> stream;

    switch (options.pipeline_mode)
    {
    case PipelineMode::Stream:
        stream = std::make_unique<StreamedSliceStream>(
            std::move(instruction_stream),
            layout,
            std::move(router),
            std::move(instruction_visitor),
            options);
        break;

    case PipelineMode::Dag:
    {
        auto dag = dag::full_dependency_dag_from_instruction_stream(*instruction_stream);
        stream = std::make_unique<DagSliceStream>(
            std::move(dag),
            instruction_stream->core_qubits(),
            layout,
            std::move(router),
            std::move(instruction_visitor),
            options);
        break;
    }

    case PipelineMode::Wave:
    case PipelineMode::EDPC:
        stream = std::make_unique<WaveSliceStream>(
            std::move(instruction_stream),
            layout,
            std::move(router),
            std::move(instruction_visitor),
            options);
        break;

    default: LSTK_UNREACHABLE;
    }

    if (options.timeout.has_value())
        stream = std::make_unique<TimeoutSliceStream>(std::move(stream), *options.timeout);

    return stream;
}


DensePatchComputationResult::DensePatchComputationResult(const DensePatchComputationResult& other)
 : ls_instructions_count_(other.ls_instructions_count_), slice_count_(other.slice_count_)
{}

}
