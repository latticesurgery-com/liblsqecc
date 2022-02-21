//
// Created by george on 2022-02-16.
//

#include <lsqecc/logical_lattice_ops/ls_instructions_parse.hpp>
#include <lsqecc/layout/ascii_layout_spec.hpp>
#include <lsqecc/patches/slices_to_json.hpp>
#include <lsqecc/patches/fast_patch_computation.hpp>

#include <argparse/argparse.h>

#include <fstream>
#include <iostream>
#include <string_view>



std::string file_to_string(std::string_view fname)
{
    std::ifstream file(fname);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return std::string{buffer.str()};
}


int main(int argc, const char* argv[])
{
    std::string prog_name{argv[0]};
    argparse::ArgumentParser parser(prog_name, "Slice LS-Instructions");
    parser.add_argument()
            .names({"-i", "--instructions"})
            .description("File name of file with instructions")
            .required(true);
    parser.add_argument()
            .names({"-l", "--layout"})
            .description("File name of file with layout spec. Defaults to simple layout if none is provided")
            .required(false);
    parser.add_argument()
            .names({"-o", "--output"})
            .description("File name of output file")
            .required(true);
    parser.enable_help();

    auto err = parser.parse(argc, argv);
    if (err)
    {
        std::cout << err << std::endl;
        return -1;
    }


    lsqecc::LogicalLatticeComputation computation {
        lsqecc::parse_ls_instructions(file_to_string(parser.get<std::string>("i")))};

    std::unique_ptr<lsqecc::Layout> layout;
    if(parser.exists("l"))
        layout = std::make_unique<lsqecc::LayoutFromSpec>(file_to_string(parser.get<std::string>("l")));
    else
        layout = std::make_unique<lsqecc::SimpleLayout>(computation.core_qubits.size());


    lsqecc::PatchComputation patch_computation {computation, std::move(layout)};
    auto slices_json = lsqecc::computation_to_json(patch_computation);
    //std::cout << slices_json.dump(3) << std::endl;
    std::ofstream(parser.get<std::string>("o")) << slices_json.dump(3) << std::endl;



    return 0;
}