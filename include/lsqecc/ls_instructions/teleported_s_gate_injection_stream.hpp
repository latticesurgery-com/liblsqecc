#ifndef LSQECC_TELEPORTED_S_GATE_INJECTION_STREAM_HPP
#define LSQECC_TELEPORTED_S_GATE_INJECTION_STREAM_HPP

#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
#include <lsqecc/layout/dynamic_layouts/determine_exposed_operators.hpp>

namespace lsqecc
{

// TODO InjectionStream -> LLIPass
class TeleportedSGateInjectionStream : public LSInstructionStream
{
public:
    TeleportedSGateInjectionStream(
            std::unique_ptr<LSInstructionStream>&& source,
            IdGenerator& id_generator,
            bool always_rotate
            );

    LSInstruction get_next_instruction() override;

    bool has_next_instruction() const override;

    const tsl::ordered_set<PatchId> &core_qubits() const override;

private:

    std::unique_ptr<LSInstructionStream> source_;
    std::queue<LSInstruction> next_instructions_;
    IdGenerator& id_generator_;
    bool always_rotate_;
};

}

#endif //LSQECC_TELEPORTED_S_GATE_INJECTION_STREAM_HPP
