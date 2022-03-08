
#include <lstk/lstk.hpp>

#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
#include <lsqecc/ls_instructions/ls_instructions_parse.hpp>



namespace lsqecc {
using namespace std::string_literals;


void LSInstructionStream::advance_instruction()
{
    std::string line;
    while(!instructions_file_.eof() && line.size() ==0)
    {
        std::getline(instructions_file_, line);
        line_number_++;
    }

    if(line.size()==0)
    {
        next_instruction_ = std::nullopt;
        return;
    }

    next_instruction_ = [&](){
        try {
            return parse_ls_instruction(std::string_view{line});
        } catch (const InstructionParseException& e) {
            throw std::runtime_error{
                    "Encountered parsing exception at line "s+std::to_string(line_number_)+":\n"s+e.what()};
        }
    }();
}


LSInstructionStream::LSInstructionStream(std::istream& instructions_file)
    :instructions_file_(instructions_file)
{

    advance_instruction();
    if(!next_instruction_)
        throw std::runtime_error("No instructions");

    LSInstruction first_instruction = get_next_instruction();
    if(!std::holds_alternative<DeclareLogicalQubitPatches>(first_instruction.operation))
        throw std::runtime_error("First instruction must be qubit declaration");

    core_qubits_ = std::get<DeclareLogicalQubitPatches>(first_instruction.operation).patch_ids;
}

LSInstruction LSInstructionStream::get_next_instruction()
{
    LSInstruction instruction = next_instruction_.value();
    advance_instruction();
    return instruction;
}


}

