#ifndef LSQECC_LS_INSTRUCTION_STREAM_HPP
#define LSQECC_LS_INSTRUCTION_STREAM_HPP

#include <lsqecc/ls_instructions/ls_instructions.hpp>
#include <lsqecc/ls_instructions/ls_instructions_from_gates.hpp>
#include <lsqecc/ls_instructions/id_generator.hpp>
#include <lsqecc/gates/parse_gates.hpp>


#include <tsl/ordered_set.h>

#include <optional>
#include <istream>
#include <iterator>
#include <cstddef>
#include <queue>

namespace lsqecc {



class LSInstructionStream
{
public:
    virtual LSInstruction get_next_instruction() =0 ;
    virtual bool has_next_instruction() const = 0;
    virtual const tsl::ordered_set<PatchId>& core_qubits() const = 0;

    virtual ~LSInstructionStream(){};
};



class LSInstructionStreamFromFile : public LSInstructionStream {
public:
    explicit LSInstructionStreamFromFile(std::istream& instructions_file);

    LSInstruction get_next_instruction() override;
    bool has_next_instruction() const override {return next_instruction_.has_value();};
    const tsl::ordered_set<PatchId>& core_qubits() const override {return core_qubits_;}

private:
    std::istream& instructions_file_;
    std::optional<LSInstruction> next_instruction_;
    tsl::ordered_set<PatchId> core_qubits_;
    size_t line_number_ = 0;

    void advance_instruction();
};


class LSInstructionStreamFromGateStream : public LSInstructionStream {
public:
    LSInstructionStreamFromGateStream(
            GateStream& gate_stream, // TODO why is this not rvalue ref?
            CNOTCorrectionMode cnot_correction_mode,
            IdGenerator& id_generator
            );

    LSInstruction get_next_instruction() override;
    bool has_next_instruction() const override {return next_instructions_.size() || gate_stream_.has_next_gate();};
    const tsl::ordered_set<PatchId>& core_qubits() const override;

private:

    GateStream& gate_stream_;
    std::queue<LSInstruction> next_instructions_;
    tsl::ordered_set<PatchId> core_qubits_;
    LSIinstructionFromGatesGenerator instruction_generator_;
    CNOTCorrectionMode cnot_correction_mode_;
};


std::ostream& print_all_ls_instructions_to_string(std::ostream& os, std::unique_ptr<LSInstructionStream>&& ls_instruction_stream);


}

#endif //LSQECC_LS_INSTRUCTION_STREAM_HPP
