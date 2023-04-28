#ifndef DECOMPOSE_ROTATION_STREAM
#define DECOMPOSE_ROTATION_STREAM

#include <lsqecc/gates/parse_gates.hpp>
#include <memory>

namespace lsqecc
{

class DecomposeRotationStream : public GateStream
{
public:
    explicit DecomposeRotationStream(std::unique_ptr<GateStream>&& input_gate_stream, double rz_precision_log_ten_negative) 
     :input_gate_stream_(std::move(input_gate_stream)), rz_precision_log_ten_negative_(rz_precision_log_ten_negative) {}

    gates::Gate get_next_gate() override;
    bool has_next_gate() const override;
    const Qreg& get_qreg() const override {return input_gate_stream_->get_qreg();};
private:

    std::unique_ptr<GateStream> input_gate_stream_;
    double rz_precision_log_ten_negative_;
    std::queue<gates::Gate> next_gates_;
};

}

#endif // DECOMPOSE_ROTATION_STREAM