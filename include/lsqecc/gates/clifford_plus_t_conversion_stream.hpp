#ifndef CLIFFORD_PLUS_T_CONVERSION_STREAM
#define CLIFFORD_PLUS_T_CONVERSION_STREAM

#include <lsqecc/gates/parse_gates.hpp>
#include <memory>

namespace lsqecc
{

class CliffordPlusTConversionStream : public GateStream
{
public:
    explicit CliffordPlusTConversionStream(std::unique_ptr<GateStream>&& input_gate_stream) 
     :input_gate_stream_(std::move(input_gate_stream)) {}

    gates::Gate get_next_gate() override;
    bool has_next_gate() const override;
    const Qreg& get_qreg() const override {return input_gate_stream_->get_qreg();};
private:

    std::unique_ptr<GateStream> input_gate_stream_;
    std::queue<gates::Gate> next_gates_;
};

}

#endif // CLIFFORD_PLUS_T_CONVERSION_STREAM