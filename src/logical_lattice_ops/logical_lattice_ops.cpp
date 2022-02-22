#include <lsqecc/logical_lattice_ops/logical_lattice_ops.hpp>

#include <ranges>


namespace lsqecc {


std::vector<PatchId> LogicalLatticeOperation::get_operating_patches() const
{
    std::vector<PatchId> ret;
    if (const auto* s = std::get_if<SinglePatchMeasurement>(&operation))
    {
        ret.push_back(s->target);
    }
    else if (const auto* p = std::get_if<SingleQubitOp>(&operation))
    {
        ret.push_back(p->target);
    }
    else if (const auto* m = std::get_if<MultiPatchMeasurement>(&operation))
    {
        for(auto pair : m->observable)
        {
            ret.push_back(pair.first);
        }
    }
    else {
        const auto& mr = std::get<MagicStateRequest>(operation);
        ret.push_back(mr.target);
    }
    return ret;
}




}