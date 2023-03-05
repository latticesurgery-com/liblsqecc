#pragma once

namespace lsqecc::dag
{

/**
 * Instructions need to implement this trait to take advantage of parallelism in a dependancy dag
 */
template <typename Instruction>
struct CommutationTrait
{
    // Requirement:
    // Instructions commute -> maybe true
    // Instructions don't commute -> definitely false
    static bool can_commute(const Instruction& a, const Instruction& b);
};

} // namespace lsqecc::dag
