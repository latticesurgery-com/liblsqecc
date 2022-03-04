

#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
#include <lsqecc/ls_instructions/ls_instructions_parse.hpp>



namespace lsqecc {

LSInstructionStream::LSInstructionStream(std::ifstream&& instructions_file)
    :instructions_file_(std::move(instructions_file))
{
    if(!has_next_instruction())
        throw std::runtime_error("No instructions");

    LSInstruction first_instruction = get_next_instruction();
    if(!std::holds_alternative<DeclareLogicalQubitPatches>(first_instruction.operation))
        throw std::runtime_error("First instruction must be qubit declaration");

    core_qubits_ = std::get<DeclareLogicalQubitPatches>(first_instruction.operation).patch_ids;
}

LSInstruction LSInstructionStream::get_next_instruction()
{
    std::string line;
    std::getline(instructions_file_, line);
    return parse_ls_instruction(std::string_view{line});
}

bool LSInstructionStream::has_next_instruction() const
{
    return !instructions_file_.eof();
}

}

