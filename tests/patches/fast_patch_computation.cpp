#include <gtest/gtest.h>

#include <lsqecc/patches/patch_computation.hpp>

using namespace lsqecc;


TEST(PatchComputation, make)
{
    InMemoryLogicalLatticeComputation assembly{
        .core_qubits = {0},
        .instructions = {
        }
    };

#if false
    PatchComputation p(assembly);

    ASSERT_EQ(1,p.num_slices());

    Slice expected{
            .distance_dependant_timesteps=1,
            .qubit_patches=std::vector<Patch>{
                    Patch{
                            .cells=SingleCellOccupiedByPatch{
                                    .top={BoundaryType::Rough, false},
                                    .bottom={BoundaryType::Rough, false},
                                    .left={BoundaryType::Smooth, false},
                                    .right={BoundaryType::Smooth, false},
                                    .cell=Cell{0, 0}
                            },
                            .type=PatchType::Qubit,
                            .id=0,
                    }
            }
    };
    ASSERT_EQ(expected, p.last_slice());
#endif
}


TEST(MultiPatchMeasurement, get_operating_patches)
{
    MultiPatchMeasurement m{ .observable=tsl::ordered_map<PatchId, PauliOperator>{
            {0,PauliOperator::X},
            {10,PauliOperator::Y},
            {20,PauliOperator::Z}
    },
            .is_negative=false
    };

    auto op_patches = LSInstruction{m}.get_operating_patches();
    ASSERT_EQ(0,op_patches[0]);
    ASSERT_EQ(10,op_patches[1]);
    ASSERT_EQ(20,op_patches[2]);

}