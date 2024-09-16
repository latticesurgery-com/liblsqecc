#include <lsqecc/ls_instructions/boundary_rotation_injection_stream.hpp>

namespace lsqecc
{


BoundaryRotationInjectionStream::BoundaryRotationInjectionStream(
        std::unique_ptr<LSInstructionStream> &&source,
        tsl::ordered_map<PatchId, RotatableSingleQubitPatchExposedOperators>&& starting_exposed_operators)
        : source_(std::move(source)), exposed_operators_(std::move(starting_exposed_operators))
{}

BoundaryRotationInjectionStream::BoundaryRotationInjectionStream(std::unique_ptr<LSInstructionStream> &&source,
                                                                 const lsqecc::Layout &layout)
: BoundaryRotationInjectionStream(
        std::move(source),
        determine_exposed_core_operators(layout, source->core_qubits()))
{}


bool BoundaryRotationInjectionStream::has_next_instruction() const
{
    return !next_instructions_.empty() || source_->has_next_instruction();
}

const tsl::ordered_set<PatchId> &BoundaryRotationInjectionStream::core_qubits() const
{
    return source_->core_qubits();
}

LSInstruction BoundaryRotationInjectionStream::get_next_instruction()
{
    if(!next_instructions_.empty()) return lstk::queue_pop(next_instructions_);

    if(!source_->has_next_instruction())
            throw std::logic_error{"BoundaryRotationInjectionStream: No more instructions from source"};

    LSInstruction new_instruction = source_->get_next_instruction();
    if(const auto* mpm = std::get_if<MultiPatchMeasurement>(&new_instruction.operation))
    {
        for(const PatchId patch_id : new_instruction.get_operating_patches())
        {
            if(exposed_operators_.contains(patch_id) && !exposed_operators_.at(patch_id).is_exposed(mpm->observable.at(patch_id)))
            {
                next_instructions_.push({RotateSingleCellPatch{patch_id}});
                exposed_operators_.at(patch_id).rotate();
            }
        }
    } 
    if (const auto* bell_cnot = std::get_if<BellBasedCNOT>(&new_instruction.operation))
    {
        if (exposed_operators_.contains(bell_cnot->control) && !exposed_operators_.at(bell_cnot->control).is_exposed(PauliOperator::Z))
        {
            next_instructions_.push({RotateSingleCellPatch{bell_cnot->control}});
            exposed_operators_.at(bell_cnot->control).rotate();
        }

        if (exposed_operators_.contains(bell_cnot->target) && !exposed_operators_.at(bell_cnot->target).is_exposed(PauliOperator::X))
        {
            next_instructions_.push({RotateSingleCellPatch{bell_cnot->target}});
            exposed_operators_.at(bell_cnot->target).rotate();
        }
 
    }
    else if (const auto* sq_gate = std::get_if<SingleQubitOp>(&new_instruction.operation))
    {
        if (exposed_operators_.contains(sq_gate->target))
        {
            if (sq_gate->op == SingleQubitOp::Operator::H)
            {
                exposed_operators_.at(sq_gate->target).rotate();
            }
            else if ((sq_gate->op == SingleQubitOp::Operator::S && 
                    !exposed_operators_.at(sq_gate->target).is_exposed(PauliOperator::Z)))
            {
                next_instructions_.push({RotateSingleCellPatch{sq_gate->target}});
                exposed_operators_.at(sq_gate->target).rotate();                
            }
        } 

    }
    
    next_instructions_.push(new_instruction);

    return lstk::queue_pop(next_instructions_);
}

}