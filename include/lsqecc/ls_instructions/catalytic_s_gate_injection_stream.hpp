#ifndef LSQECC_CATALYTIC_S_GATE_INJECTION_STREAM_HPP
#define LSQECC_CATALYTIC_S_GATE_INJECTION_STREAM_HPP

#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
#include <lsqecc/layout/dynamic_layouts/determine_exposed_operators.hpp>
#include <lsqecc/ls_instructions/ls_instructions_from_gates.hpp>

namespace lsqecc
{

class CatalyticSGateInjectionStream : public LSInstructionStream
{
public:
    CatalyticSGateInjectionStream(
            std::unique_ptr<LSInstructionStream>&& source,
            IdGenerator& id_generator,
            bool local_instruction
            );

    LSInstruction get_next_instruction() override;

    bool has_next_instruction() const override;

    const tsl::ordered_set<PatchId> &core_qubits() const override;

private:

    std::unique_ptr<LSInstructionStream> source_;
    std::queue<LSInstruction> next_instructions_;
    IdGenerator& id_generator_;
    LSIinstructionFromGatesGenerator instruction_generator_;
};

}

#endif //LSQECC_CATALYTIC_S_GATE_INJECTION_STREAM_HPP
