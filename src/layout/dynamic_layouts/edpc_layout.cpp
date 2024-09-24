#include <lsqecc/layout/dynamic_layouts/edpc_layout.hpp>

#include <cmath>
#include <iostream>


namespace lsqecc
{

std::unique_ptr<Layout> make_edpc_layout(size_t num_core_qubits, size_t num_lanes, bool condensed, bool factories_explicit, bool predistilled, const DistillationOptions& distillation_options)
{

    // Provides extra padding on the grid to accomodate distillation regions
    size_t t_distillation_region_cols = 0;
    size_t t_distillation_region_rows = 0;
    if (factories_explicit)
    {
        t_distillation_region_cols = 5;
        t_distillation_region_rows = 5;
    }

    // Calculate number of columns total
    // (If factories_explicit, we consider a layout with 15-to-1 distillation regions all along the boundaries)
    size_t sr = static_cast<size_t>(std::ceil(sqrt(num_core_qubits)));
    size_t core_cols = sr + num_lanes*(sr-1) + 2*num_lanes + 2;
    if (condensed) 
    {
        // for every pair of logical qubits (both row-wise and column-wise), we remove a row/column
        core_cols -= num_lanes*std::floor(sr/2);
    }

    size_t core_rows = core_cols;
    size_t dist_cols = 2*t_distillation_region_cols;
    size_t dist_rows = 2*t_distillation_region_rows;
    size_t total_cols = core_cols + dist_cols;
    size_t total_rows = core_rows + dist_rows;

    // The grid is a vector of CellRow, which is itself a vector of CellType, which is an enum
    AsciiLayoutSpec::CellGrid grid{
            total_rows, AsciiLayoutSpec::CellRow{total_cols, AsciiLayoutSpec::CellType::RoutingAncilla}};

    // Loop through tiles within the bulk and keep counter of number of logical qubits placed
    size_t logical_placed = 0;
    bool condense_shift_vert = 0;
    bool condense_shift_horiz = 0;
    int rows_lost = 0;
    for (size_t i = t_distillation_region_rows; i<grid.size()-t_distillation_region_rows; i++) 
    {
        bool row_lost = 0;
        int cols_lost = 0;
        for (size_t j = t_distillation_region_cols; j<grid[i].size()-t_distillation_region_cols; j++)
        {
            // Place logical qubit patches and tiles reserved for resource states
            if ((i-t_distillation_region_rows-((1-condense_shift_vert)*rows_lost+condense_shift_vert)*(num_lanes+1+(1-condense_shift_vert)))%(num_lanes+1+condense_shift_vert)==0 && 
                (j-t_distillation_region_cols-((1-condense_shift_horiz)*cols_lost+condense_shift_horiz)*(num_lanes+1+(1-condense_shift_horiz)))%(num_lanes+1+condense_shift_horiz)==0) 
            {
                if (i == t_distillation_region_rows || j == t_distillation_region_cols 
                    || i == grid.size()-t_distillation_region_rows -1 || j == grid[i].size() -t_distillation_region_cols -1) 
                {
                    grid[i][j] = AsciiLayoutSpec::CellType::PreDistilledYState;

                    if (condensed) 
                    {
                        if ((j > t_distillation_region_cols) && (j < grid[i].size()- t_distillation_region_cols - (num_lanes+1) - 1))
                        {
                            grid[i][j+1] = AsciiLayoutSpec::CellType::PreDistilledYState;
                            condense_shift_horiz = 1;
                            cols_lost++;
                        }
                        if ((i > t_distillation_region_rows) && (i < grid.size() - t_distillation_region_rows - (num_lanes+1) - 1))
                        {
                            grid[i+1][j] = AsciiLayoutSpec::CellType::PreDistilledYState;
                            condense_shift_vert = 1;
                            row_lost = 1;
                        }  

                        if (j >= grid[i].size()- t_distillation_region_cols - (num_lanes+1) - 1) {condense_shift_horiz = 0;}
                    }
                }
                else 
                {
                    if (logical_placed < num_core_qubits) 
                    {
                        grid[i][j] = AsciiLayoutSpec::CellType::LogicalComputationQubit_StandardBorderOrientation;
                        logical_placed++;
                        
                        if (condensed) 
                        {
                            if ((logical_placed < num_core_qubits) && (j < grid[i].size()- t_distillation_region_cols - (num_lanes+1) - 1))
                            {
                                grid[i][j+1] = AsciiLayoutSpec::CellType::LogicalComputationQubit_StandardBorderOrientation;
                                logical_placed++;
                                condense_shift_horiz = 1;
                                cols_lost++;
                            }
                            if ((logical_placed < num_core_qubits) && (i < grid.size() - t_distillation_region_rows - (num_lanes+1) - 1))
                            {
                                grid[i+1][j] = AsciiLayoutSpec::CellType::LogicalComputationQubit_StandardBorderOrientation;
                                logical_placed++;
                                condense_shift_vert = 1;
                                row_lost = 1;
                            }
                            if ((logical_placed < num_core_qubits) && (j < grid[i].size()- t_distillation_region_cols - (num_lanes+1) - 1) &&
                                (i < grid.size() - t_distillation_region_rows - (num_lanes+1) - 1))
                            {
                                grid[i+1][j+1] = AsciiLayoutSpec::CellType::LogicalComputationQubit_StandardBorderOrientation;
                                logical_placed++;
                            }

                            if (j >= grid[i].size()- t_distillation_region_cols - (num_lanes+1) - 1) {condense_shift_horiz = 0;}
                            if (i >= grid.size() - t_distillation_region_rows - (num_lanes+1) - 1) {condense_shift_vert = 0;}
                        }
                    }
                    else 
                    {
                        if (j >= grid[i].size()- t_distillation_region_cols - (num_lanes+1) - 1) {condense_shift_horiz = 0;}
                        if (i >= grid.size() - t_distillation_region_rows - (num_lanes+1) - 1) {condense_shift_vert = 0;}
                    }
                }
            }
        }
        rows_lost += row_lost;
    }

    // Add distillation blocks on north edge
    size_t a_count = 0;
    size_t i = t_distillation_region_rows;
    for (size_t j = t_distillation_region_cols; j<grid[i].size()-t_distillation_region_cols; j++) {
        if (grid[i][j] == 'Y') {
            a_count++;
            if (a_count%2 == 0) {
                if (!factories_explicit)
                    grid[i][j] = AsciiLayoutSpec::CellType::ReservedForMagicState;
                else 
                    grid[i][j] = AsciiLayoutSpec::CellType::RoutingAncilla;

                for (size_t k=i-t_distillation_region_rows; k<i; k++) {
                    for (size_t l=j-1; l<j+2; l++) {
                        grid[k][l] = AsciiLayoutSpec::CellType::DistillationRegion_0;
                    }
                }
            }
            else if (!predistilled) {
                if (!factories_explicit) 
                    grid[i][j] = AsciiLayoutSpec::CellType::ReservedForMagicState;
                else
                    grid[i][j] = AsciiLayoutSpec::CellType::RoutingAncilla;  
            }
        }
    }

    // Add distillation blocks on south edge
    a_count = 0;
    i = grid.size() - t_distillation_region_rows - 1;
    for (size_t j = t_distillation_region_cols; j<grid[i].size()-t_distillation_region_cols; j++) {
        if (grid[i][j] == 'Y') {
            a_count++;
            if (a_count%2 == 0) {
                if (!factories_explicit)
                    grid[i][j] = AsciiLayoutSpec::CellType::ReservedForMagicState;
                else 
                    grid[i][j] = AsciiLayoutSpec::CellType::RoutingAncilla;

                for (size_t k=i+1; k<i+t_distillation_region_rows+1; k++) {
                    for (size_t l=j-1; l<j+2; l++) {
                        grid[k][l] = AsciiLayoutSpec::CellType::DistillationRegion_0;
                    }
                }
            }
            else if (!predistilled) {
                if (!factories_explicit) 
                    grid[i][j] = AsciiLayoutSpec::CellType::ReservedForMagicState;
                else
                    grid[i][j] = AsciiLayoutSpec::CellType::RoutingAncilla;  
            }
        }
    }

    // Add distillation blocks on west edge
    a_count = 0;
    size_t j = t_distillation_region_cols;
    for (size_t i = t_distillation_region_rows; i<grid.size()-t_distillation_region_rows; i++) {
        if (grid[i][j] == 'Y') {
            a_count++;
            if (a_count%2 == 0) {
                if (!factories_explicit)
                    grid[i][j] = AsciiLayoutSpec::CellType::ReservedForMagicState;
                else 
                    grid[i][j] = AsciiLayoutSpec::CellType::RoutingAncilla;

                for (size_t k=i-1; k<i+2; k++) {
                    for (size_t l=j-t_distillation_region_cols; l<j; l++) {
                        grid[k][l] = AsciiLayoutSpec::CellType::DistillationRegion_0;
                    }
                }
            }
            else if (!predistilled) {
                if (!factories_explicit) 
                    grid[i][j] = AsciiLayoutSpec::CellType::ReservedForMagicState;
                else
                    grid[i][j] = AsciiLayoutSpec::CellType::RoutingAncilla;  
            }
        }
    }

    // Add distillation blocks on east edge
    a_count = 0;
    j = grid[0].size() - t_distillation_region_cols - 1;
    for (size_t i = t_distillation_region_rows; i<grid.size()-t_distillation_region_rows; i++) {
        if (grid[i][j] == 'Y') {
            a_count++;
            if (a_count%2 == 0) {
                if (!factories_explicit)
                    grid[i][j] = AsciiLayoutSpec::CellType::ReservedForMagicState;
                else 
                    grid[i][j] = AsciiLayoutSpec::CellType::RoutingAncilla;

                for (size_t k=i-1; k<i+2; k++) {
                    for (size_t l=j+1; l<j+t_distillation_region_cols+1; l++) {
                        grid[k][l] = AsciiLayoutSpec::CellType::DistillationRegion_0;
                    }
                }
            }
            else if (!predistilled) {
                if (!factories_explicit) 
                    grid[i][j] = AsciiLayoutSpec::CellType::ReservedForMagicState;
                else
                    grid[i][j] = AsciiLayoutSpec::CellType::RoutingAncilla;  
            }
        }
    }

    // Add dead cells
    for (size_t i = 0; i<grid.size(); i++) {
        for (size_t j = 0; j<grid[i].size(); j++) {
            if (i >= t_distillation_region_rows && i < grid.size() - t_distillation_region_rows && j >= t_distillation_region_cols && j < grid[i].size() - t_distillation_region_cols) {
                continue;
            }
            else if (grid[i][j] == 'r') {
                grid[i][j] = AsciiLayoutSpec::CellType::DeadCell;
            }
        }
    }

#ifdef DEBUG_EDPC_LAYOUT_CREATION
    // Print out the grid for the purposes of debugging
    std::cerr << "The number of rows in the grid is: " << grid.size() << std::endl;
    std::cerr << "The number of columns in the grid is: " << grid[grid.size()-1].size() << std::endl;
    for (size_t i=0; i<grid.size(); i++) {
        for (size_t j=0; j<grid[i].size(); j++) {
            std::cerr << grid[i][j];
        }
        std::cerr << std::endl;
    }
#endif

    return std::make_unique<LayoutFromSpec>(grid, distillation_options);
}

} // namespace lsqecc
