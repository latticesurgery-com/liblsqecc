#include <lsqecc/dependency_dag/lli_dag.hpp>
#include <unordered_set>

namespace lsqecc {

LLIDag make_dag_from_instruction_stream(std::unique_ptr<LSInstructionStream>&& lli_stream)
{
    LLIDag lli_dag;

    while(lli_stream->has_next_instruction())
        lli_dag.push(lli_stream->get_next_instruction());

    return lli_dag;
}


template<> // TODO replace with concepts
struct CommutationTrait<LSInstruction> {
    static bool may_not_commute(const LSInstruction& lhs, const LSInstruction& rhs)
    {
        const auto lhs_operating_patches{lhs.get_operating_patches()};
        const auto rhs_operating_patches{rhs.get_operating_patches()};
        std::unordered_set<PatchId> lhs_set{LSTK_RANGE(lhs_operating_patches)};
        std::unordered_set<PatchId> rhs_set{LSTK_RANGE(rhs_operating_patches)};
        return !lstk::set_intersection(lhs_set, rhs_set).empty();
    }
};

}
