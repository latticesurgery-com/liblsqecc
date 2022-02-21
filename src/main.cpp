//
// Created by george on 2022-02-16.
//

#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>


#include <lsqecc/logical_lattice_ops/ls_instructions_parse.hpp>
#include <lsqecc/layout/ascii_layout_spec.hpp>
#include <lsqecc/patches/slices_to_json.hpp>
#include <lsqecc/patches/fast_patch_computation.hpp>


std::string file_to_string(std::string_view fname)
{
    std::ifstream file(fname);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return std::string{buffer.str()};
}


int main()
{
    lsqecc::LogicalLatticeComputation computation {
        lsqecc::parse_ls_instructions(file_to_string("test_ls_instructions.txt"))};

    auto layout = std::make_unique<lsqecc::LayoutFromSpec>(file_to_string("test_layout.txt"));


    lsqecc::PatchComputation patch_computation {computation, std::move(layout)};
    auto slices_json = lsqecc::computation_to_json(patch_computation);
    //std::cout << slices_json.dump(3) << std::endl;
    std::ofstream("f1.json") << slices_json.dump(3) << std::endl;
}