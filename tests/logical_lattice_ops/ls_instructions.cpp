#include <gtest/gtest.h>

#include <lsqecc/ls_instructions/ls_instructions.hpp>

using namespace lsqecc;

TEST(SinglePatchMeasurement, get_operating_patches)
{
    LSInstruction op{
        .operation=SinglePatchMeasurement{
            .target=99,
            .observable=PauliOperator::X,
            .is_negative=false
        }
    };

    ASSERT_EQ(1,op.get_operating_patches().size());
    ASSERT_EQ(99,op.get_operating_patches().at(0));
}

TEST(SingleQubitOp, get_operating_patches)
{
    LSInstruction op{
        .operation=SingleQubitOp{
            .target=98,
            .op=SingleQubitOp::Operator::H
        }
    };

    ASSERT_EQ(1,op.get_operating_patches().size());
    ASSERT_EQ(98,op.get_operating_patches().at(0));
}

TEST(MultiPatchMeasurement, get_operating_patches__one_patch)
{
    LSInstruction op{
        .operation=MultiPatchMeasurement{
            .observable={{static_cast<PatchId>(11), PauliOperator::X}},
            .is_negative=false
        }
    };

    ASSERT_EQ(1,op.get_operating_patches().size());
    ASSERT_EQ(11,op.get_operating_patches().at(0));
}



TEST(MultiPatchMeasurement, get_operating_patches__many_patches)
{
    LSInstruction op{
        .operation=MultiPatchMeasurement{
            .observable={
                        {static_cast<PatchId>(11),PauliOperator::X},
                        {static_cast<PatchId>(12),PauliOperator::Z},
                        {static_cast<PatchId>(13),PauliOperator::X},
                        {static_cast<PatchId>(14),PauliOperator::Z},
                    },
            .is_negative=false
        }
    };

    ASSERT_EQ(4,op.get_operating_patches().size());
    ASSERT_EQ(11,op.get_operating_patches().at(0));
    ASSERT_EQ(12,op.get_operating_patches().at(1));
    ASSERT_EQ(13,op.get_operating_patches().at(2));
    ASSERT_EQ(14,op.get_operating_patches().at(3));
}
