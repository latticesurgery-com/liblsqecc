#ifndef LSQECC_POSTGRES_GATE_STREAM_HPP
#define LSQECC_POSTGERS_GATE_STREAM_HPP

#include <lsqecc/gates/parse_gates.hpp>
#include <lsqecc/patches/patches.hpp>

#include <queue>
#include <memory>

namespace pq {
    struct connection;
}

namespace lsqecc {

class PandoraPostgresGateStream : public GateStream
{
public:
    PandoraPostgresGateStream(std::string host, std::string port, std::string dbname);
    gates::Gate get_next_gate() override;
    bool has_next_gate() const override;
    const Qreg& get_qreg() const override;
    const tsl::ordered_set<PatchId>& core_qubits() const;

    ~PandoraPostgresGateStream(); // needed to work with the incomplete type pq::connection

private:
    mutable std::unique_ptr<pq::connection> db_;
    const Qreg qreg_;
    mutable std::queue<gates::Gate> gates_in_layer_;
    mutable size_t current_layer_ = 0;

    void advance_layer() const;
};

}
#endif