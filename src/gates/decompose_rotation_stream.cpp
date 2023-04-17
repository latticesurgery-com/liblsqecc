#include <lsqecc/gates/decompose_rotation_stream.hpp>


namespace lsqecc
{

bool DecomposeRotationStream::has_next_gate() const
{
    return !next_gates_.empty() || input_gate_stream_->has_next_gate();
}

gates::Gate DecomposeRotationStream::get_next_gate()
{
    if(!next_gates_.empty()) return lstk::queue_pop(next_gates_);

    if(!input_gate_stream_->has_next_gate()) LSTK_UNREACHABLE;

    gates::Gate next_gate{input_gate_stream_->get_next_gate()};
    if(gates::is_decomposed(next_gate)) return next_gate;

    auto decomposed = gates::decompose(next_gate, rz_precision_log_ten_negative_);
    for (const auto& gate : decomposed)
        next_gates_.push(gate);
    
    return lstk::queue_pop(next_gates_);
}

}