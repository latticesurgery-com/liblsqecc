
#include <lsqecc/patches/slices_to_json.hpp>

using namespace nlohmann;

namespace lsqecc {

json init_blank_json_slice(const Slice& slice)
{
    json out_rows = json::array();
    Cell furthest_cell = slice.get_layout().furthest_cell();
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


void annotate_time_to_next_distilled_state(json& json_slice, const Slice& slice)
{
    size_t distillation_region_counter = 0;
    for(const MultipleCellsOccupiedByPatch& distillation_region: slice.get_layout().distillation_regions())
    {
        const auto distillation_cell = distillation_region.sub_cells.front().cell;
        json_slice[distillation_cell.row][distillation_cell.col]["text"] = std::string{"Time to next magic state:"} + std::to_string(
                slice.time_to_next_magic_state(distillation_region_counter++));
    }
}


json boundaries_to_array_edges_json(const CellBoundaries& cell_boundaries)
{
    auto orientation_to_json = [](Boundary boundary){
        switch (boundary.boundary_type)
        {
        case BoundaryType::None:return "None";
        case BoundaryType::Connected: return boundary.is_active ? "AncillaJoin": "None";
        case BoundaryType::Rough:return boundary.is_active ? "DashedStiched": "Dashed";
        case BoundaryType::Smooth: return boundary.is_active ? "SolidStiched": "Solid";
        // case BoundaryType::Reserved: throw std::logic_error("BoundaryType::Reserved was not converted to activity in local compilation.");
        case BoundaryType::Reserved_Label1: return boundary.is_active ? "SolidStiched": "None";
        case BoundaryType::Reserved_Label2: return boundary.is_active ? "DashedStiched": "None";
        }
        LSTK_UNREACHABLE;
    };

    return json{{"edges", {
        {"Top", orientation_to_json(cell_boundaries.top)},
        {"Bottom", orientation_to_json(cell_boundaries.bottom)},
        {"Left", orientation_to_json(cell_boundaries.left)},
        {"Right", orientation_to_json(cell_boundaries.right)}
    }}};
}


json dense_patch_to_json(const DensePatch& p)
{
    json visual_array_cell = boundaries_to_array_edges_json(p.boundaries);
        if(p.operation_id)
            visual_array_cell["operation_id"] = std::to_string(*p.operation_id);

    visual_array_cell["patch_type"] = [&](){
        switch (p.type)
        {
        case PatchType::Distillation: return "DistillationQubit";
        case PatchType::PreparedState:return "DistillationQubit";
        case PatchType::Qubit: return "Qubit";
        case PatchType::Routing: return "Ancilla";
        case PatchType::Dead: return "Ancilla";
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
                        case PatchActivity::Distillation: return json();
                        case PatchActivity::Dead: return json();
                        case PatchActivity::MultiPatchMeasurement: return json();
                        case PatchActivity::Rotation: return json();
                        case PatchActivity::EDPC: return json();
                        case PatchActivity::Reserved: return json();
                        }
                        LSTK_UNREACHABLE;
                    }()
            }
    };

    if(p.id)
        visual_array_cell["text"] = std::string{"Id: "} + std::to_string(*p.id);
    else if ((p.type==PatchType::Distillation && p.activity == PatchActivity::None) ||  p.type ==PatchType::Qubit)
        visual_array_cell["text"] = "Not bound";
    else
        visual_array_cell["text"] = "";
    return visual_array_cell;
}


json slice_to_json(const DenseSlice& slice)
{
    json out_slice = init_blank_json_slice(slice);
    slice.traverse_cells([&](const Cell& c, const std::optional<DensePatch>& p) {
        if(p) out_slice[c.row][c.col] = dense_patch_to_json(*p);
    });

    annotate_time_to_next_distilled_state(out_slice, slice);

    return out_slice;
}


json slice_to_json(const SparseSlice& slice)
{
    json out_slice = init_blank_json_slice(slice);

    std::vector<SparsePatch> all_patches{slice.qubit_patches};
    all_patches.insert(all_patches.end(),slice.unbound_magic_states.begin(), slice.unbound_magic_states.end());
    for(const SparsePatch& p : all_patches)
    {
        if(auto single_cell_patch = std::get_if<SingleCellOccupiedByPatch>(&p.cells))
        {
            out_slice[single_cell_patch->cell.row][single_cell_patch->cell.col] =
                    dense_patch_to_json(DensePatch::from_sparse_patch(p));
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
            json visual_array_cell = boundaries_to_array_edges_json(routing_cell);
            visual_array_cell["patch_type"] = "Ancilla";
            visual_array_cell["activity"] = json({});
            if(routing_region.routing_region_id)
                visual_array_cell["operation_id"] =  std::to_string(*routing_region.routing_region_id);
            out_slice[routing_cell.cell.row][routing_cell.cell.col] = visual_array_cell;
        }
    }

    for(const MultipleCellsOccupiedByPatch& distillation_region: slice.layout.get().distillation_regions())
    {
        for(const SingleCellOccupiedByPatch& distillation_cell: distillation_region.sub_cells)
        {
            json visual_array_cell = boundaries_to_array_edges_json(distillation_cell);
            visual_array_cell["patch_type"] = "DistillationQubit";
            visual_array_cell["activity"] = json({});
        }
    }
    annotate_time_to_next_distilled_state(out_slice, slice);

    return out_slice;
}



}