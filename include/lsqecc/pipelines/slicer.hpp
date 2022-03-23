#ifndef LSQECC_SLICER_HPP
#define LSQECC_SLICER_HPP

#include <iostream>

namespace lsqecc {

int run_slicer_program(
        int argc, const char* argv[],
        std::istream& in_stream,
        std::ostream& out_stream,
        std::ostream& err_stream);



std::string run_slicer_program_from_strings(std::string command_line, std::string standard_input);

struct LsqeccClass {
    std::string call_run_slicer_program_from_strings(std::string command_line, std::string standard_input)
    {
        return run_slicer_program_from_strings(command_line, standard_input);
    }
};


}


#endif //LSQECC_SLICER_HPP
