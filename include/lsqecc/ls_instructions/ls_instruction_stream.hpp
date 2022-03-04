#ifndef LSQECC_LS_INSTRUCTION_STREAM_HPP
#define LSQECC_LS_INSTRUCTION_STREAM_HPP

#include <lsqecc/ls_instructions/ls_instructions.hpp>

#include <tsl/ordered_set.h>

#include <fstream>
#include <iterator>
#include <cstddef>

namespace lsqecc {

class LSInstructionStream;



class LSInstructionStream {
public:
    LSInstructionStream(std::ifstream&& instructions_file);

    LSInstruction get_next_instruction();
    bool has_next_instruction() const;
    const tsl::ordered_set<PatchId>& core_qubits() const {return core_qubits_;}

private:
    std::ifstream instructions_file_;
    tsl::ordered_set<PatchId> core_qubits_;
    size_t line_number_ = 0;
};

}

#endif //LSQECC_LS_INSTRUCTION_STREAM_HPP
