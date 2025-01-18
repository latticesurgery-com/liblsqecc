#include <lsqecc/gates/postgres_gate_stream.hpp>

#include <pqpp/pq.hpp>

enum class PandoraGateTranslator {
    In = 0,
    Out,
    Rx,
    Ry,
    Rz,
    XPowGate,
    YPowGate,
    ZPowGate,
    HPowGate,
    _PauliX,
    _PauliZ,
    _PauliY,
    GlobalPhaseGate,
    ResetChannel,
    M,
    CNOT,
    CZ,
    CZPowGate,
    CXPowGate,
    XXPowGate,
    ZZPowGate,
    Toffoli,
    And,
    CCXPowGate
};



namespace lsqecc {

PandoraPostgresGateStream::PandoraPostgresGateStream(std::string host, std::string port, std::string dbname)
    : db_(std::make_unique<pq::connection>("host=" + host + " port=" + port + " dbname=" + dbname))
    , qreg_(Qreg{"pandora_reg", [&]() -> QubitNum {
        auto r = db_->exec("select count(*) from layered_cliff_t where layered_cliff_t.type=0;");
        auto count = r.at(0).at("count").get<int64_t>();
        if(count > std::numeric_limits<QubitNum>::max())
            throw std::runtime_error("Too many qubits in the pandora database");
        return static_cast<QubitNum>(count);
    }()})
{
    current_layer_++; // The first layer onl has inits, so we skip it
}
    
using PostgresBigIntApprox = int64_t;

void PandoraPostgresGateStream::advance_layer() const {
    auto r = db_->exec("select * from layered_cliff_t where layered_cliff_t.layer=" + std::to_string(current_layer_) + ";");
    for (auto& row : r) {
        auto gate =[&]() -> std::optional<gates::Gate> {
            switch(static_cast<PandoraGateTranslator>(row["type"].get<int>())) {
                // Only handle supported gates:
                // TODO here we are pretending X and Y gates are actually Z, for simplicity. This is not correct, but in 
                // The future, Pandora will be configured to only output gates supported by lsqecc.
                case PandoraGateTranslator::XPowGate:
                case PandoraGateTranslator::YPowGate:
                case PandoraGateTranslator::ZPowGate:
                {
                    double angle = row["param"].get<double>();
                    if (std::abs(angle) == 1) {
                        return gates::Z(row["target_q"].get<PostgresBigIntApprox>());
                    } else if (angle == 0.5) {
                        return gates::S(row["target_q"].get<PostgresBigIntApprox>());
                    } else if (angle == -0.5) {
                        return gates::SDg(row["target_q"].get<PostgresBigIntApprox>());
                    } else if (angle == 0.25) {
                        return gates::T(row["target_q"].get<PostgresBigIntApprox>());
                    } else if (angle == -0.25) {
                        return gates::TDg(row["target_q"].get<PostgresBigIntApprox>());
                    } else {
                        throw std::runtime_error(lstk::cat("Unsupported angle for ZPowGate: ", angle));
                    }
                }
                case PandoraGateTranslator::HPowGate:
                    return gates::H(row["target_q"].get<PostgresBigIntApprox>());
                case PandoraGateTranslator::_PauliX:
                    return gates::X(row["target_q"].get<PostgresBigIntApprox>());
                case PandoraGateTranslator::_PauliZ:
                    return gates::Z(row["target_q"].get<PostgresBigIntApprox>());
                case PandoraGateTranslator::Out:
                case PandoraGateTranslator::In:
                case PandoraGateTranslator::M: // TODO implement
                case PandoraGateTranslator::ResetChannel: // TODO implement
                    return std::nullopt;
                case PandoraGateTranslator::CZPowGate:  // TODO: pretending this is a CNOT
                case PandoraGateTranslator::CXPowGate: // Assume the exponent is 1
                    return gates::CNOT(row["control_q"].get<PostgresBigIntApprox>(),row["target_q"].get<PostgresBigIntApprox>());
                default:
                    throw std::runtime_error(lstk::cat("Unsupported gate type: ", row["type"].get<PostgresBigIntApprox>()));
            }
        }();
        if(gate) gates_in_layer_.push(*gate);
    }
    ++current_layer_;
}

gates::Gate PandoraPostgresGateStream::get_next_gate()
{
    if (gates_in_layer_.empty()) advance_layer();

    auto gate = gates_in_layer_.front();
    gates_in_layer_.pop();
    return gate;
}

bool PandoraPostgresGateStream::has_next_gate() const
{
    if (gates_in_layer_.empty())
        advance_layer();
    
    return !gates_in_layer_.empty();

}

const Qreg& PandoraPostgresGateStream::get_qreg() const {
    return qreg_;
}

PandoraPostgresGateStream::~PandoraPostgresGateStream() = default;

}