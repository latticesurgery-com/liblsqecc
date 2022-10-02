#ifndef LSQECC_BOUNDARY_ROTATION_INJECTION_STREAM_HPP
#define LSQECC_BOUNDARY_ROTATION_INJECTION_STREAM_HPP

#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
#include <lsqecc/layout/dynamic_layouts/determine_exposed_operators.hpp>

namespace lsqecc
{

class BoundaryRotationInjectionStream : public LSInstructionStream
{
public:
    BoundaryRotationInjectionStream(std::unique_ptr<LSInstructionStream>&& source, const Layout& layout);

    BoundaryRotationInjectionStream(
            std::unique_ptr<LSInstructionStream>&& source,
            tsl::ordered_map<PatchId, RotatableSingleQubitPatchExposedOperators>&& starting_exposed_operators);

    LSInstruction get_next_instruction() override;

    bool has_next_instruction() const override;

    const tsl::ordered_set<PatchId> &core_qubits() const override;

private:

    std::unique_ptr<LSInstructionStream> source_;
    std::queue<LSInstruction> next_instructions_;
    tsl::ordered_map<PatchId, RotatableSingleQubitPatchExposedOperators> exposed_operators_;
};

}

#endif //LSQECC_BOUNDARY_ROTATION_INJECTION_STREAM_HPP
