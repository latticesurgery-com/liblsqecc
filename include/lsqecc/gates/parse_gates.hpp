#ifndef LSQECC_PARSE_GATES_HPP
#define LSQECC_PARSE_GATES_HPP

#include <lsqecc/gates/gates.hpp>

#include <pqxx/pqxx>

#include <string_view>
#include <istream>
#include <vector>
#include <optional>
#include <iterator>
#include <cstddef>

namespace lsqecc {


struct GateParseException : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};


// indicate that the instruction is something like an OPENQASM or include standard gates directive
struct IgnoredInstruction{};

struct Qreg
{
    std::string name;
    QubitNum size;
};

using ParseGateResult = std::variant<gates::Gate, IgnoredInstruction, Qreg>;
ParseGateResult parse_gate(std::string_view str_line);


class GateStream
{
public:
    virtual gates::Gate get_next_gate() =0 ;
    virtual bool has_next_gate() const = 0;
    virtual const Qreg& get_qreg() const = 0;

    virtual ~GateStream() {};
};



class GateStreamFromFile : public GateStream
{
public:
    explicit GateStreamFromFile(std::istream& gate_file);

    gates::Gate get_next_gate() override;
    bool has_next_gate() const override {return next_gate_.has_value();};
    const Qreg& get_qreg() const override {return qreg_.value();};
private:

    std::istream& gate_file_;
    std::optional<gates::Gate> next_gate_;
    std::optional<Qreg> qreg_;
    size_t line_number_ = 0;

    void advance_gate();
};


class GateStreamFromStreamFromDB : public GateStream
{
public:
    explicit GateStreamFromStreamFromDB(std::string_view db_conn_string);

    gates::Gate get_next_gate() override;
    bool has_next_gate() const override;
    const Qreg& get_qreg() const override;
private:
    // TODO implement has_next_gate with a lookup instead, so these don't need to 
    // be mutable
    mutable pqxx::connection db_connection_;
    Qreg qreg_;
    size_t max_moment;

    mutable size_t current_offset_ = 0;    
    mutable std::deque<gates::Gate> gates_batch_;
    
    const size_t k_offset_increase = 100000;

    void advance_moment_cache() const;
};


}



#endif //LSQECC_PARSE_GATES_HPP
