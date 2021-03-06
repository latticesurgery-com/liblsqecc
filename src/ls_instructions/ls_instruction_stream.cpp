
#include <lstk/lstk.hpp>

#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
#include <lsqecc/ls_instructions/ls_instructions_parse.hpp>
#include <lsqecc/ls_instructions/from_gates.hpp>




namespace lsqecc {
using namespace std::string_literals;


void LSInstructionStreamFromFile::advance_instruction()
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


LSInstructionStreamFromFile::LSInstructionStreamFromFile(std::istream& instructions_file)
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

LSInstruction LSInstructionStreamFromFile::get_next_instruction()
{
    LSInstruction instruction = next_instruction_.value();
    advance_instruction();
    return instruction;
}

LSInstruction LSInstructionStreamFromGateStream::get_next_instruction()
{
    if(next_instructions_.empty())
    {
        if(!gate_stream_.has_next_gate())
            throw std::logic_error{"LSInstructionStreamFromGateStream: No more gates but requesting instructions"};

        // RESUME HERE BREAKING INSTRUCIONS DOWN AND PUSHING THEM TO next_instructions_

    }

    return lstk::queue_pop(next_instructions_);
}



LSInstructionStreamFromGateStream::LSInstructionStreamFromGateStream(GateStream& gate_stream)
: gate_stream_(gate_stream)
{
    for(PatchId id = 0; id <gate_stream_.get_qreg().size; id++)
        core_qubits_.insert(id);
}
const tsl::ordered_set<PatchId>& LSInstructionStreamFromGateStream::core_qubits() const
{
    return core_qubits_;
}

}

