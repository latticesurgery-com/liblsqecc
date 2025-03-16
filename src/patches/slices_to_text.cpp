#include "lsqecc/patches/slices_to_text.hpp"
#include <sstream>

namespace lsqecc {

// Helper: determine the label for a given cell.
// Here we assume:
// - If the cell is empty, label "route".
// - If the cell has a patch:
//      * For Qubit: if activity is Measurement, label "measurement"; else "qubit".
//      * For Ancilla or Routing: label "ancilla".
//      * For Distillation: label "distillation".
//      * For other types: label "unknown".
std::string get_cell_label(const std::optional<DensePatch>& patch)
{
    if (!patch.has_value()) {
        return "route";
    }
    // Determine the label based on patch type and activity.
    switch (patch->type) {
    case PatchType::Qubit:
        return (patch->activity == PatchActivity::Measurement) ? "measurement"
                                                               : "qubit";
    case PatchType::Routing:
        return "ancilla";
    case PatchType::Distillation:
        return "distillation";
    // Other cases here.
    default:
        return "unknown";
    }
}

// Converts a single DenseSlice to text.
std::string slice_to_text(const DenseSlice& slice, size_t time_stamp)
{
    std::ostringstream oss;
    // Get grid dimensions from the layout.
    Cell furthest = slice.get_layout().furthest_cell();
    for (int row = 0; row <= furthest.row; ++row) {
        for (int col = 0; col <= furthest.col; ++col) {
            // Get the cell coordinates.
            Cell cell = Cell::from_ints(row, col);
            // Get the patch (if any) from the slice.
            const auto& patch = slice.patch_at(cell);
            // Determine the label.
            std::string label = get_cell_label(patch);
            // Print the coordinate in the format (row,col,time) followed by the label.
            oss << "(" << row << "," << col << "," << time_stamp << ") "
                << label << "\n";
        }
    }
    return oss.str();
}

// Converts a vector of DenseSlices (representing time slices) to text.
std::string slices_to_text(const std::vector<DenseSlice>& slices)
{
    std::ostringstream oss;
    for (size_t t = 0; t < slices.size(); ++t) {
        oss << slice_to_text(slices[t], t) << "\n";
    }
    return oss.str();
}

} // namespace lsqecc