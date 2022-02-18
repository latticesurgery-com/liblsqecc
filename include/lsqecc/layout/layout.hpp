#ifndef LSQECC_LAYOUT_HPP
#define LSQECC_LAYOUT_HPP


#include <lsqecc/patches/patches.hpp>

namespace lsqecc {

using SurfaceCodeTimestep = uint32_t;


struct Layout {
    virtual const std::vector<Patch> core_patches() const = 0;
    virtual Cell min_furthest_cell() const = 0;

    virtual std::vector<MultipleCellsOccupiedByPatch> distillation_regions() const = 0;
    virtual std::vector<SurfaceCodeTimestep> distillation_times() const = 0;
};



class SimpleLayout : public Layout {
public:
    explicit SimpleLayout(size_t num_qubits) : num_qubits_(num_qubits) {}

    const std::vector<Patch> core_patches() const override;
    Cell min_furthest_cell() const override {
        return Cell{4,6}; // Could extract from distillation region;
    }

    std::vector<SurfaceCodeTimestep> distillation_times() const override {
        return {10,10};
    }

    std::vector<MultipleCellsOccupiedByPatch> distillation_regions() const override
    {
        return {
            MultipleCellsOccupiedByPatch{.sub_cells={
                    distillation_region_cell(Cell{2,0}),distillation_region_cell(Cell{2,1}),distillation_region_cell(Cell{2,2}),
                    distillation_region_cell(Cell{3,0}),distillation_region_cell(Cell{3,1}),distillation_region_cell(Cell{3,2}),
                    distillation_region_cell(Cell{4,0}),distillation_region_cell(Cell{4,1}),distillation_region_cell(Cell{4,2}),
            }},
            MultipleCellsOccupiedByPatch{.sub_cells={
                    distillation_region_cell(Cell{2,4}),distillation_region_cell(Cell{2,5}),distillation_region_cell(Cell{2,6}),
                    distillation_region_cell(Cell{3,4}),distillation_region_cell(Cell{3,5}),distillation_region_cell(Cell{3,6}),
                    distillation_region_cell(Cell{4,4}),distillation_region_cell(Cell{4,5}),distillation_region_cell(Cell{4,6}),
            }},
        };
    }

private:
    static Patch basic_square_patch(Cell placement){
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


    static SingleCellOccupiedByPatch distillation_region_cell(Cell placement){
        return SingleCellOccupiedByPatch{
                        .top={BoundaryType::Connected,false},
                        .bottom={BoundaryType::Connected,false},
                        .left={BoundaryType::Connected,false},
                        .right={BoundaryType::Connected,false},
                        .cell=placement
                };

    }

private:
    size_t num_qubits_;


};


}


#endif //LSQECC_LAYOUT_HPP
