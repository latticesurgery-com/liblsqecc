#include <lsqecc/layout/dynamic_layouts/compact_layout.hpp>

#include <cmath>

namespace lsqecc
{


std::unique_ptr<Layout> make_compact_layout(size_t num_core_qubits, const DistillationOptions& distillation_options)
{
    size_t t_distillation_region_cols = 5;
    auto core_cols = static_cast<size_t>(std::ceil(static_cast<double>(num_core_qubits)/2.0));
    size_t non_core_cols = /* S distillation + routing */ 3 + t_distillation_region_cols;
    size_t total_cols = core_cols + non_core_cols;

    AsciiLayoutSpec::CellGrid grid{
            3, AsciiLayoutSpec::CellRow{total_cols, AsciiLayoutSpec::CellType::RoutingAncilla}};

    // Fill the core qubits
    std::fill_n(grid.at(0).begin(), core_cols,
            AsciiLayoutSpec::CellType::LogicalComputationQubit_StandardBorderOrientation);
    std::fill_n(grid.at(2).begin(), num_core_qubits-core_cols,
            AsciiLayoutSpec::CellType::LogicalComputationQubit_StandardBorderOrientation);

    // Add the distillation regions
    grid.at(0).at(core_cols+1) = AsciiLayoutSpec::CellType::AncillaQubitLocation;
    grid.at(2).at(core_cols+1) = AsciiLayoutSpec::CellType::AncillaQubitLocation;
    std::fill_n(grid.at(0).begin() + static_cast<long>(core_cols) + 3, t_distillation_region_cols,
    AsciiLayoutSpec::CellType::DistillationRegion_0);
    std::fill_n(grid.at(1).begin() + static_cast<long>(core_cols) + 3, t_distillation_region_cols,
    AsciiLayoutSpec::CellType::DistillationRegion_0);
    std::fill_n(grid.at(2).begin() + static_cast<long>(core_cols) + 3, t_distillation_region_cols,
    AsciiLayoutSpec::CellType::DistillationRegion_0);

    return std::make_unique<LayoutFromSpec>(grid, distillation_options);
}

} // namespace lsqecc
