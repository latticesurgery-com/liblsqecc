#include <lsqecc/ls_instructions/ls_instructions.hpp>

#include <sstream>
#include <vector>


namespace lsqecc {

namespace LocalInstruction 
{

std::ostream& operator<<(std::ostream& os, const LocalLSInstruction& instruction)
{
    std::visit([&os](auto&& op){ os << op;}, instruction.operation);
    return os;
}

std::ostream& operator<<(std::ostream& os, const BellPrepare& instruction)
{
    os << LocalInstructionPrint<BellPrepare>::name
        << " " << instruction.cell1 << "," << instruction.cell2;

    return os;
}
std::ostream& operator<<(std::ostream& os, const BellMeasure& instruction)
{
    os << LocalInstructionPrint<BellMeasure>::name
        << " " << instruction.cell1 << "," << instruction.cell2;

    return os;
}
std::ostream& operator<<(std::ostream& os, const TwoPatchMeasure& instruction)
{
    os << LocalInstructionPrint<TwoPatchMeasure>::name
        << " " << instruction.cell1 << "," << instruction.cell2;

    return os;
}
std::ostream& operator<<(std::ostream& os, const ExtendSplit& instruction)
{
    os << LocalInstructionPrint<ExtendSplit>::name
        << " " << instruction.target_cell << "," << instruction.extension_cell;

    return os;
}
std::ostream& operator<<(std::ostream& os, const Move& instruction)
{
    os << LocalInstructionPrint<Move>::name
        << " " << instruction.cell1 << "," << instruction.cell2;

    return os;
}
}


}