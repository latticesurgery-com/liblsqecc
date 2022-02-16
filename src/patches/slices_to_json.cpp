
#include <lsqecc/patches/slices_to_json.hpp>


using namespace nlohmann;

namespace lsqecc {


json init_blank_json_slice(const Slice& slice)
{
    json out_rows = json::array();
    Cell furthest_cell = slice.get_furthest_cell();
    for (int row_idx = 0; row_idx<=furthest_cell.row; ++row_idx)
    {
        json out_row = json::array();
        for (int col_idx = 0; col_idx<=furthest_cell.col; ++col_idx)
        {
            out_row.push_back(json::array());
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
        case BoundaryType::Rough:return boundary.is_active ? "Dashed": "DashedStiched";
        case BoundaryType::Smooth: return boundary.is_active ? "Solid": "SolidStiched";
        }
    };

    return json{{"edges", {
        {"Top", orientation_to_json(cell.top)},
        {"Bottom", orientation_to_json(cell.bottom)},
        {"Left", orientation_to_json(cell.left)},
        {"Right", orientation_to_json(cell.right)}
    }}};
}

json slices_to_json(const std::vector<Slice>& slices)
{
    json out_slices = json::array();

    for(const Slice& slice : slices)
    {
        json out_slice = init_blank_json_slice(slice);

        for(const Patch& p : slice.patches)
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
                }();
                visual_array_cell["activity"] = json();
                visual_array_cell["text"] = "";
                out_slice[single_cell_patch->row][single_cell_patch->col] = visual_array_cell;
            }
            else
            {
                const auto& multi_cell_patch = std::get<MultipleCellsOccupiedByPatch>(p.cells);
                // TODO
            }
        }
        out_slices.push_back(out_slice);
    }
    return out_slices;
}

}