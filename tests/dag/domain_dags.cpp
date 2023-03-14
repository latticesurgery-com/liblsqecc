#include <gtest/gtest.h>

#include <lsqecc/dag/domain_dags.hpp>

using namespace lsqecc;

#define COMMA ,

TEST(commutation_trait, can_commute)
{
    ASSERT_TRUE(dag::CommutationTrait<gates::Gate>::can_commute(gates::X(0) COMMA gates::X(1)));
    ASSERT_FALSE(dag::CommutationTrait<gates::Gate>::can_commute(gates::X(0) COMMA gates::X(0)));

    ASSERT_TRUE(dag::CommutationTrait<gates::Gate>::can_commute(gates::CNOT(0 COMMA 1) COMMA gates::CNOT(2 COMMA 3))); // All different
    ASSERT_TRUE(dag::CommutationTrait<gates::Gate>::can_commute(gates::CNOT(0 COMMA 2) COMMA gates::CNOT(1 COMMA 2))); // Same control
    ASSERT_FALSE(dag::CommutationTrait<gates::Gate>::can_commute(gates::CNOT(0 COMMA 1) COMMA gates::CNOT(0 COMMA 2))); // Same target
    ASSERT_FALSE(dag::CommutationTrait<gates::Gate>::can_commute(gates::CNOT(0 COMMA 1) COMMA gates::CNOT(0 COMMA 1))); // All the same
}
