#include <lsqecc/ls_instructions/ls_instructions.hpp>



namespace lsqecc {


std::vector<PatchId> LSInstruction::get_operating_patches() const
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
    else if (const auto* rotation = std::get_if<RotateSingleCellPatch>(&operation)) {
        ret.push_back(rotation->target);
    }
    else {
        const auto& mr = std::get<MagicStateRequest>(operation);
        ret.push_back(mr.target);
    }
    return ret;
}

std::ostream& operator<<(std::ostream& os, const LSInstruction& instruction)
{
    return os << "<An LS Instruction>";
}


}