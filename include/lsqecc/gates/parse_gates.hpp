#ifndef LSQECC_PARSE_GATES_HPP
#define LSQECC_PARSE_GATES_HPP

#include <lsqecc/gates/gates.hpp>

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


}



#endif //LSQECC_PARSE_GATES_HPP
