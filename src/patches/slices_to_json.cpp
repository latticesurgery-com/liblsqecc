
#include <lsqecc/patches/slices_to_json.hpp>

using namespace nlohmann;

namespace lsqecc {


json init_blank_json_slice(const SparseSlice& slice)
{
    json out_rows = json::array();
    Cell furthest_cell = slice.layout.get().furthest_cell();
    for (int row_idx = 0; row_idx<=furthest_cell.row; ++row_idx)
    {
        json out_row = json::array();
        for (int col_idx = 0; col_idx<=furthest_cell.col; ++col_idx)
        {
            out_row.push_back(json());
        }
        out_rows.push_back(out_row);
    }

    return out_rows;
}



json cell_patch_to_visual_array_edges_json(const SingleCellOccupiedByPatch& cell)
{
    auto orientation_to_json = [](Boundary boundary){
        switch (boundary.boundary_type)
        {
        case BoundaryType::None:return "None";
        case BoundaryType::Connected: return boundary.is_active ? "AncillaJoin": "None";
        case BoundaryType::Rough:return boundary.is_active ? "DashedStiched": "Dashed";
        case BoundaryType::Smooth: return boundary.is_active ? "SolidStiched": "Solid";
        }
        LSTK_UNREACHABLE;
    };

    return json{{"edges", {
        {"Top", orientation_to_json(cell.top)},
        {"Bottom", orientation_to_json(cell.bottom)},
        {"Left", orientation_to_json(cell.left)},
        {"Right", orientation_to_json(cell.right)}
    }}};
}


json core_to_json(const SparseSlice& slice)
{
    json out_slice = init_blank_json_slice(slice);

    std::vector<SparsePatch> all_patches{slice.qubit_patches};
    all_patches.insert(all_patches.end(),slice.unbound_magic_states.begin(), slice.unbound_magic_states.end());
    for(const SparsePatch& p : all_patches)
    {
        if(auto single_cell_patch = std::get_if<SingleCellOccupiedByPatch>(&p.cells))
        {
            json visual_array_cell = cell_patch_to_visual_array_edges_json(*single_cell_patch);
            visual_array_cell["patch_type"] = [&](){
                switch (p.type)
                {
                case PatchType::Distillation:
                case PatchType::PreparedState:return "DistillationQubit";
                case PatchType::Qubit: return "Qubit";
                case PatchType::Routing: return "Ancilla";
                }
                LSTK_UNREACHABLE;
            }();

            visual_array_cell["activity"] = {
                    {
                            "activity_type",
                            [&](){
                                switch (p.activity)
                                {
                                case PatchActivity::None: return json();
                                case PatchActivity::Measurement: return json("Measurement");
                                case PatchActivity::Unitary: return json("Unitary");
                                }
                                LSTK_UNREACHABLE;
                            }()
                    }
            };

            visual_array_cell["text"] = p.id ? std::string{"Id: "} + std::to_string(*p.id) : "Not bound";
            out_slice[single_cell_patch->cell.row][single_cell_patch->cell.col] = visual_array_cell;
        }
        else
        {
            const auto& multi_cell_patch = std::get<MultipleCellsOccupiedByPatch>(p.cells);
            LSTK_UNUSED(multi_cell_patch);
            throw std::runtime_error{"MultipleCellsOccupiedByPatch to JSON not implemented"};
        }
    }

    for(const RoutingRegion& routing_region: slice.routing_regions)
    {
        for(const SingleCellOccupiedByPatch& routing_cell: routing_region.cells)
        {
            json visual_array_cell = cell_patch_to_visual_array_edges_json(routing_cell);
            visual_array_cell["patch_type"] = "Ancilla";
            visual_array_cell["activity"] = json({});
            // TODO could add sanity check on indices
            out_slice[routing_cell.cell.row][routing_cell.cell.col] = visual_array_cell;
        }
    }

    size_t distillation_region_counter = 0;
    for(const MultipleCellsOccupiedByPatch& distillation_region: slice.layout.get().distillation_regions())
    {
        bool placed_ttd = false;
        for(const SingleCellOccupiedByPatch& distillation_cell: distillation_region.sub_cells)
        {
            json visual_array_cell = cell_patch_to_visual_array_edges_json(distillation_cell);
            visual_array_cell["patch_type"] = "DistillationQubit";
            visual_array_cell["activity"] = json({});
            if(!placed_ttd)
            {
                visual_array_cell["text"] = std::string{"Time to next magic state:"} + std::to_string(
                        slice.time_to_next_magic_state_by_distillation_region[distillation_region_counter]);
                placed_ttd = true;
            }
            // TODO could add sanity check on indices
            out_slice[distillation_cell.cell.row][distillation_cell.cell.col] = visual_array_cell;
        }

        distillation_region_counter++;
    }

    return out_slice;
}

json slices_to_json(const std::vector<SparseSlice>& slices)
{
    json out_slices = json::array();
    
    for(const SparseSlice& slice : slices)
        out_slices.push_back(core_to_json(slice));


    return out_slices;
}

}