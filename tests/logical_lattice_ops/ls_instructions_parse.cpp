#include <gtest/gtest.h>

#include <lsqecc/logical_lattice_ops/ls_instructions_parse.hpp>

using namespace lsqecc;

TEST(parse_ls_instructions, parse_declare_qubit_ids)
{
    auto llops = parse_ls_instructions("DeclareLogicalQubitPatches 10234,11,1001,1002,1009,1010,1");
    ASSERT_EQ(llops.instructions.size(), 0);
    ASSERT_EQ(llops.core_qubits.size(), 7);

    for(auto p : std::vector<PatchId>{10234, 11, 1001, 1002, 1009, 1010, 1})
        ASSERT_TRUE(llops.core_qubits.contains(p));
}

TEST(parse_ls_instructions, one_single_patch_measurement_x)
{
    auto llops = parse_ls_instructions("MeasureSinglePatch X 1");
    ASSERT_EQ(llops.instructions.size(),1);
    LogicalLatticeOperation op{SinglePatchMeasurement{1, PauliOperator::X, false}};
    ASSERT_EQ(llops.instructions[0],op);
    ASSERT_EQ(llops.core_qubits,std::unordered_set<PatchId>{});
}

TEST(parse_ls_instructions, one_single_patch_measurement_z)
{
    auto llops = parse_ls_instructions("MeasureSinglePatch Z 1001");
    ASSERT_EQ(llops.instructions.size(),1);
    LogicalLatticeOperation op{SinglePatchMeasurement{1001, PauliOperator::Z, false}};
    ASSERT_EQ(llops.instructions[0],op);
    ASSERT_EQ(llops.core_qubits,std::unordered_set<PatchId>{});
}


TEST(parse_ls_instructions, one_multibody_measurement_z)
{
    auto llops = parse_ls_instructions("MultiBodyMeasure 1001:Z");
    ASSERT_EQ(llops.instructions.size(),1);
    LogicalLatticeOperation op{MultiPatchMeasurement{tsl::ordered_map<PatchId, PauliOperator>{{1001, PauliOperator::Z}}, false}};
    ASSERT_EQ(llops.instructions[0],op);
    ASSERT_EQ(llops.core_qubits,std::unordered_set<PatchId>{});
}


TEST(parse_ls_instructions, one_multibody_measurement_zzxx)
{
    auto llops = parse_ls_instructions("MultiBodyMeasure 1001:Z,1002:Z,1009:X,1010:X");
    ASSERT_EQ(llops.instructions.size(),1);
    LogicalLatticeOperation op{MultiPatchMeasurement{
        tsl::ordered_map<PatchId, PauliOperator>{
                {1001, PauliOperator::Z},
                {1002, PauliOperator::Z},
                {1009, PauliOperator::X},
                {1010, PauliOperator::X},
            },false}};
    ASSERT_EQ(llops.instructions[0],op);
    ASSERT_EQ(llops.core_qubits,std::unordered_set<PatchId>{});
}

TEST(parse_ls_instructions, one_magic_state_request)
{
    auto llops = parse_ls_instructions("RequestMagicState 11");
    ASSERT_EQ(llops.instructions.size(),1);
    LogicalLatticeOperation op{MagicStateRequest{11}};
    ASSERT_EQ(llops.instructions[0],op);
    ASSERT_EQ(llops.core_qubits,std::unordered_set<PatchId>{});
}


TEST(parse_ls_instructions, one_logical_pauli)
{
    auto llops = parse_ls_instructions("LogicalPauli Z 10234");
    ASSERT_EQ(llops.instructions.size(),1);
    LogicalLatticeOperation op{LogicalPauli{10234, PauliOperator::Z}};
    ASSERT_EQ(llops.instructions[0],op);
    ASSERT_EQ(llops.core_qubits,std::unordered_set<PatchId>{});
}

TEST(parse_ls_instructions, one_of_each)
{
    auto llops = parse_ls_instructions(
            "DeclareLogicalQubitPatches 10234,11,1001,1002,1009,1010,1\n"
            "LogicalPauli Z 10234\n"
            "RequestMagicState 11\n"
            "MultiBodyMeasure 1001:Z,1002:Z,1009:X,1010:X\n"
            "\n"
            "\n"
            "MeasureSinglePatch Z 1\n"
            );
    ASSERT_EQ(llops.instructions.size(),4);
    std::vector<LogicalLatticeOperation> ops = {
            LogicalLatticeOperation{LogicalPauli{10234, PauliOperator::Z}},
            LogicalLatticeOperation{MagicStateRequest{11}},
            LogicalLatticeOperation{MultiPatchMeasurement{
                        tsl::ordered_map<PatchId, PauliOperator>{
                                {1001, PauliOperator::Z},
                                {1002, PauliOperator::Z},
                                {1009, PauliOperator::X},
                                {1010, PauliOperator::X},
                        },false}},
            LogicalLatticeOperation{SinglePatchMeasurement{1, PauliOperator::Z, false}}
            };
    ASSERT_EQ(llops.instructions,ops);
    std::unordered_set<PatchId> ids{1001,1002,1009,1010,10234,11,1};
    ASSERT_EQ(llops.core_qubits,ids);
}


TEST(parse_ls_instructions, blank_lines)
{
    auto llops = parse_ls_instructions("\n\n\n\n");
    ASSERT_EQ(llops.instructions.size(),0);
    ASSERT_EQ(llops.core_qubits,std::unordered_set<PatchId>{});
}


