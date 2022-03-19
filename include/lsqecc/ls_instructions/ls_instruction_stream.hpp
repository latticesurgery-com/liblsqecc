#ifndef LSQECC_LS_INSTRUCTION_STREAM_HPP
#define LSQECC_LS_INSTRUCTION_STREAM_HPP

#include <lsqecc/ls_instructions/ls_instructions.hpp>

#include <tsl/ordered_set.h>

#include <optional>
#include <fstream>
#include <fstream>
#include <iterator>
#include <cstddef>

namespace lsqecc {

class LSInstructionStream
{
public:
    virtual LSInstruction get_next_instruction() =0 ;
    virtual bool has_next_instruction() const = 0;
    virtual const tsl::ordered_set<PatchId>& core_qubits() const = 0;
};



class LSInstructionStreamFromFile : public LSInstructionStream {
public:
    LSInstructionStreamFromFile(std::istream& instructions_file);

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

}

#endif //LSQECC_LS_INSTRUCTION_STREAM_HPP
