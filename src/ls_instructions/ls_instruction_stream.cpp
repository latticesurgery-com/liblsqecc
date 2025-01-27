#include <lstk/lstk.hpp>

#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
#include <lsqecc/ls_instructions/ls_instructions_parse.hpp>




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

    LSInstruction first_instruction = next_instruction_.value();
    advance_instruction();
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

        gates::Gate next_gate = gate_stream_.get_next_gate();

        if(const auto* sq_gate = std::get_if<gates::BasicSingleQubitGate>(&next_gate))
        {   
            switch(sq_gate->gate_type)
            {
#define SINGLE_QUBIT_OP_CASE(Op)\
                case gates::BasicSingleQubitGate::Type::Op: \
                    next_instructions_.push({.operation={SingleQubitOp{sq_gate->target_qubit,SingleQubitOp::Operator::Op}}});\
                    break;
                SINGLE_QUBIT_OP_CASE(Z)
                SINGLE_QUBIT_OP_CASE(X)
                SINGLE_QUBIT_OP_CASE(Y)
                SINGLE_QUBIT_OP_CASE(H)
                SINGLE_QUBIT_OP_CASE(S)
#undef SINGLE_QUBIT_OP_CASE
                
                case gates::BasicSingleQubitGate::Type::SDg:
                    next_instructions_.push({.operation={SingleQubitOp{sq_gate->target_qubit,SingleQubitOp::Operator::S, true}}});\
                    break;
               
                case gates::BasicSingleQubitGate::Type::TDg:
                case gates::BasicSingleQubitGate::Type::T:
                    bool is_dagger = sq_gate->gate_type == gates::BasicSingleQubitGate::Type::TDg;
                    auto instructions = instruction_generator_.make_t_gate_instructions(sq_gate->target_qubit, is_dagger);
                    lstk::queue_extend(next_instructions_, instructions);
                    break;
            }
        }
        else if(const auto* rz_gate = std::get_if<gates::RZ>(&next_gate))
        {
            const Fraction* pi_fraction = std::get_if<Fraction>(&rz_gate->angle);
            if (pi_fraction == nullptr)
                throw std::runtime_error(lstk::cat("Cannot approximate non-fractional R_z"));
            if(pi_fraction->num == 1 && pi_fraction->den == 1)
                next_instructions_.push({.operation={SingleQubitOp{rz_gate->target_qubit,SingleQubitOp::Operator::Z}}});
            else if(pi_fraction->num == 1 && pi_fraction->den == 2)
                next_instructions_.push({.operation={SingleQubitOp{rz_gate->target_qubit,SingleQubitOp::Operator::S}}});
            else if(pi_fraction->num == 1 && pi_fraction->den == 4)
            {
                auto instructions = instruction_generator_.make_t_gate_instructions(rz_gate->target_qubit);
                lstk::queue_extend(next_instructions_, instructions);
            }

            else
                throw std::runtime_error(lstk::cat(
                        "Cannot approximate R_z(",pi_fraction->num,"/",pi_fraction->den,")"));
        }
        else if(const auto* controlled_gate = std::get_if<gates::ControlledGate>(&next_gate))
        {
            if(const auto* target_gate = std::get_if<gates::BasicSingleQubitGate>(&controlled_gate->target_gate))
            {
                if(target_gate->gate_type == gates::BasicSingleQubitGate::Type::X)
                {
                    auto instructions = instruction_generator_.make_cnot_instructions(
                            controlled_gate->control_qubit,
                            target_gate->target_qubit,
                            controlled_gate->cnot_type,
                            controlled_gate->cnot_ancilla_placement,
                            cnot_correction_mode_);
                    lstk::queue_extend(next_instructions_, instructions);
                }
                else if (target_gate->gate_type == gates::BasicSingleQubitGate::Type::Z)
                {
                    // There may be better ways to implement CZ in lattice surgery (see e.g. arXiv:1808.06709)
                    next_instructions_.push({.operation={SingleQubitOp{target_gate->target_qubit,SingleQubitOp::Operator::H}}});
                    auto instructions = instruction_generator_.make_cnot_instructions(
                            controlled_gate->control_qubit,
                            target_gate->target_qubit,
                            controlled_gate->cnot_type,
                            controlled_gate->cnot_ancilla_placement,
                            cnot_correction_mode_);
                    lstk::queue_extend(next_instructions_, instructions);
                    next_instructions_.push({.operation={SingleQubitOp{target_gate->target_qubit,SingleQubitOp::Operator::H}}});
                }
                else
                    throw std::runtime_error{lstk::cat(
                            "Gate type of index ", static_cast<size_t>(target_gate->gate_type), "Not supported for control")};
            }
            else if(const auto* target_gate = std::get_if<gates::RZ>(&controlled_gate->target_gate))
            {
                LSTK_UNUSED(target_gate);
                LSTK_NOT_IMPLEMENTED;
                // TODO implement with decompose_CRZ_gate and approximate_RZ_gate
            }

        }
        else if(const auto* reset = std::get_if<gates::Reset>(&next_gate))
        {
            if (reset->register_name != gate_stream_.get_qreg().name)
                throw std::logic_error{"Invalid 'reset' register name"};
            
            next_instructions_.push({.operation={PatchReset{reset->target_qubit}}});
        }
        else
        {
            LSTK_UNREACHABLE;
        }

    }

    return lstk::queue_pop(next_instructions_);
}



tsl::ordered_set<PatchId> core_qubits_from_gate_stream(GateStream& gate_stream)
{
    tsl::ordered_set<PatchId> core_qubits;
    for(PatchId id = 0; id <gate_stream.get_qreg().size; id++)
        core_qubits.insert(id);
    assert(core_qubits.size());
    return core_qubits;
}


LSInstructionStreamFromGateStream::LSInstructionStreamFromGateStream(
        GateStream& gate_stream,
        CNOTCorrectionMode cnot_correction_mode,
        IdGenerator& id_generator,
        bool local_instructions)
: gate_stream_(gate_stream),
  next_instructions_(),
  core_qubits_(core_qubits_from_gate_stream(gate_stream)),
  instruction_generator_(id_generator, local_instructions),
  cnot_correction_mode_(cnot_correction_mode)
{
}

const tsl::ordered_set<PatchId>& LSInstructionStreamFromGateStream::core_qubits() const
{
    return core_qubits_;
}

std::ostream& print_all_ls_instructions_to_string(std::ostream& os, std::unique_ptr<LSInstructionStream>&& ls_instruction_stream)
{
    while(ls_instruction_stream->has_next_instruction())
        os << ls_instruction_stream->get_next_instruction() << "\n";
    return os;
}



}
