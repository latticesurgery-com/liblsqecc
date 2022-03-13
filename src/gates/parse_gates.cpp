#include <lsqecc/gates/parse_gates.hpp>

#include <lstk/lstk.hpp>

#include <vector>
#include <stdexcept>

namespace lsqecc {

struct Line
{
    std::string_view instruction;
    std::vector<std::string_view> args;
};


// TODO specialize for the arbitrary precision case
template<class Integer>
Integer try_parse_int(std::string_view sv)
{
    std::string s{sv};

    errno = 0;
    char* endptr;
    Integer res = std::strtol(s.c_str(), &endptr, 10);

    if (errno != 0)
        throw std::runtime_error(lstk::cat("Could not parse ", s, "as string, strtol gave errno: ",
                                                static_cast<int>(errno)));
    if (endptr == s.c_str())
        throw std::runtime_error(lstk::cat("Could not parse ", s, ". No digits were found"));
    if (*endptr != '\0')
        throw std::runtime_error(lstk::cat("Could not parse ", s, ". Further characters after number"));

//    if(std::to_string(res) != s)
//        throw std::runtime_error(std::string{"Could not parse "} + s + "as string, atol gave "+std::to_string(res));
    return res;
}

Line split_instruction_and_args(std::string_view gate_str)
{
    if(!lstk::contains(gate_str, ' '))
        return {gate_str, {}};

    auto instr_split = lstk::split_on(gate_str,' ');

    return {instr_split[0], lstk::split_on(instr_split[1], ',')};

}

QubitNum get_index_arg(std::string_view s)
{
    return try_parse_int<QubitNum>(
            lstk::split_on(
                    lstk::split_on(s,'[').at(1),
                    ']'
            ).at(0));
}


gates::Gate parse_qasm_gate(const Line& line)
{
    if(line.instruction == "x") return gates::X(try_parse_int<QubitNum>(line.args[0]));
    if(line.instruction == "z") return gates::Z(try_parse_int<QubitNum>(line.args[0]));
    if(line.instruction == "s") return gates::S(try_parse_int<QubitNum>(line.args[0]));
    if(line.instruction == "t") return gates::T(try_parse_int<QubitNum>(line.args[0]));
    if(line.instruction == "h") return gates::H(try_parse_int<QubitNum>(line.args[0]));

    if(line.instruction.substr(0,2) == "rz")
    {
        if(line.instruction.substr(2,6) == "(pi/")
        {
            auto pi_frac_den = try_parse_int<ArbitraryPrecisionInteger>(
                    lstk::split_on(line.instruction,')').at(0));
            return gates::RZ{
                get_index_arg(line.args[0]),
                Fraction{1,pi_frac_den}};
        }
        else {
            throw std::runtime_error{lstk::cat(
                    "Can only parse pi/n for n power of 2 angles as crz args, got ",line.instruction)};
        }
    }
    if(line.instruction.substr(0,3) == "crz")
    {
        if(line.instruction.substr(3,7) == "(pi/")
        {
            auto pi_frac_den = try_parse_int<ArbitraryPrecisionInteger>(
                    lstk::split_on(line.instruction,')').at(0));
            return gates::CRZ(
                get_index_arg(line.args[1]),
                get_index_arg(line.args[0]),
                Fraction{1,pi_frac_den});
        }
        else {
            throw std::runtime_error{lstk::cat(
                    "Can only parse pi/n for n power of 2 angles as crz args, got ",line.instruction)};
        }
    }

    throw std::runtime_error{lstk::cat("Instruction not implemented ",line.instruction)};
}

bool is_ignored_instruction(std::string_view instr)
{
    if( instr == "OPENQASM" ||
        instr == "include" ||
        instr == "barrier" ||
        instr == "qreg")
        return true;
    return false;
}


std::vector<gates::Gate> parse_all_gates_from_qasm(std::istream qasm_stream)
{
    std::vector<gates::Gate> out;

    while (!qasm_stream.eof())
    {
        std::string str_line;
        while (!qasm_stream.eof() && str_line.size() == 0)
            std::getline(qasm_stream, str_line);

        if(str_line.size()>0)
        {
            Line line = split_instruction_and_args(str_line);
            if (!is_ignored_instruction(line.instruction))
                out.push_back(parse_qasm_gate(line));
        }
    }

    return out;
}

}
