//
// Created by george on 2022-02-16.
//

#include <lsqecc/logical_lattice_ops/ls_instructions_parse.hpp>
#include <lsqecc/layout/ascii_layout_spec.hpp>
#include <lsqecc/layout/router.hpp>
#include <lsqecc/patches/slices_to_json.hpp>
#include <lsqecc/patches/fast_patch_computation.hpp>

#include <lstk/lstk.hpp>

#include <argparse/argparse.h>

#include <fstream>
#include <iostream>
#include <string_view>
#include <stdexcept>
#include <filesystem>
#include <chrono>



std::string file_to_string(std::string fname)
{
    if(!std::filesystem::exists(fname))
        throw std::logic_error{std::string{"File not found:"}+fname};


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
            .description("File name of file with LS Instructions")
            .required(true);
    parser.add_argument()
            .names({"-l", "--layout"})
            .description("File name of file with layout spec. Defaults to simple layout if none is provided")
            .required(false);
    parser.add_argument()
            .names({"-o", "--output"})
            .description("File name of output file to which write a latticesurgery.com JSON of the slices")
            .required(false);
    parser.add_argument()
            .names({"-t", "--timeout"})
            .description("Set a timeout in seconds after which stop producing slices")
            .required(false);
    parser.add_argument()
            .names({"-r", "--router"})
            .description("Set a router: naive_cached (default), naive")
            .required(false);
    parser.enable_help();

    auto err = parser.parse(argc, argv);
    if (err)
    {
        std::cout << err << std::endl;
        parser.print_help();
        return -1;
    }
    if (parser.exists("help"))
    {
        parser.print_help();
        return 0;
    }


    std::cout << "Reading LS Instructions" << std::endl;
    auto start = std::chrono::steady_clock::now();
    lsqecc::InMemoryLogicalLatticeComputation logical_lattice_computation {
        lsqecc::parse_ls_instructions(file_to_string(parser.get<std::string>("i")))};
    std::cout << "Read " << logical_lattice_computation.instructions.size() << " instructions."
        << " Took " << lstk::since(start).count() << "s." << std::endl;

    std::unique_ptr<lsqecc::Layout> layout;
    if(parser.exists("l"))
        layout = std::make_unique<lsqecc::LayoutFromSpec>(file_to_string(parser.get<std::string>("l")));
    else
        layout = std::make_unique<lsqecc::SimpleLayout>(logical_lattice_computation.core_qubits.size());

    auto timeout = parser.exists("t") ?
            std::make_optional(std::chrono::seconds{parser.get<ulong>("t")})
            : std::nullopt;


    std::unique_ptr<lsqecc::Router> router = std::make_unique<lsqecc::CachedNaiveDijkstraRouter>();
    if(parser.exists("r"))
    {
        auto router_name = parser.get<std::string>("r");
        if(router_name =="naive_cached")
            LSTK_NOOP;// Already set
        else if(router_name=="naive")
            router = std::make_unique<lsqecc::NaiveDijkstraRouter>();
        else
        {
            std::cerr<<"Unknown router: "<< router_name <<std::endl;
            std::cerr << "Choices are: naive, naive_cached." << std::endl;
            return -1;
        }
    }

    auto no_op_visitor = [](const lsqecc::Slice& s) -> void {LSTK_UNUSED(s);};
    lsqecc::PatchComputation::SliceVisitorFunction slice_visitor(no_op_visitor);
    std::vector<lsqecc::Slice> slices;
    if(parser.exists("o"))
    {
        std::cout << "Will keep slices in memory during computation to write them as json" << std::endl;
        slice_visitor = [&slices](const lsqecc::Slice& s){
            slices.push_back(s);
        };
    }



    std::cout << "Making patch computation" << std::endl;
    start = std::chrono::steady_clock::now();
    lsqecc::PatchComputation patch_computation {
            logical_lattice_computation,
            std::move(layout),
            std::move(router),
            timeout,
            slice_visitor
    };

    std::cout << "Made patch computation. Took " << lstk::since(start).count() << "s." << std::endl;
    std::cout << "Generated " << patch_computation.slice_count() << " slices." << std::endl;

    if(slices.size()>0)
    {
        std::cout << "Writing slices to file" << std::endl;
        start = std::chrono::steady_clock::now();
        auto slices_json = lsqecc::slices_to_json(slices);
        std::ofstream(parser.get<std::string>("o")) << slices_json.dump(3) << std::endl;
        std::cout << "Written slices. Took "<< lstk::since(start).count() << "s." << std::endl;
    }


    return 0;
}