#include <lsqecc/logical_lattice_ops/logical_lattice_ops.hpp>

#include <ranges>


namespace lsqecc {


PauliOperator PauliOperator_from_string(std::string_view s)
{
    if (s=="I") return PauliOperator::I;
    if (s=="X") return PauliOperator::X;
    if (s=="Y") return PauliOperator::Y;
    if (s=="Z") return PauliOperator::Z;
    throw std::logic_error(std::string{"Not a PauliOperator:"}+std::string{s});
}


std::vector<PatchId> LogicalLatticeOperation::get_operating_patches() const
{
    std::vector<PatchId> ret;
    if (const auto* s = std::get_if<SinglePatchMeasurement>(&operation))
    {
        ret.push_back(s->target);
    }
    else if (const auto* p = std::get_if<LogicalPauli>(&operation))
    {
        ret.push_back(p->target);
    }
    else if (const auto* m = std::get_if<MultiPatchMeasurement>(&operation))
    {
        auto keys = std::views::transform(m->targetted_observable, [](const auto& pair){return pair.first;});
        std::ranges::copy(keys, std::back_inserter(ret));
    }
    else {
        const auto& mr = std::get<MagicStateRequest>(operation);
        ret.push_back(mr.target);
    }
    return ret;
}

}