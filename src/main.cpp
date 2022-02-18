//
// Created by george on 2022-02-16.
//

#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>


#include <lsqecc/logical_lattice_ops/ls_instructions_parse.hpp>
#include <lsqecc/patches/slices_to_json.hpp>
#include <lsqecc/patches/fast_patch_computation.hpp>

int main()
{
    std::ifstream f("test_ls_instructions.txt");
    std::stringstream buffer;
    buffer << f.rdbuf();
    std::string s{buffer.str()};
    auto computation {lsqecc::parse_ls_instructions(s)};
    auto patch_computation {lsqecc::PatchComputation::make(computation)};
    auto slices_json = lsqecc::computation_to_json(patch_computation);
    //std::cout << slices_json.dump(3) << std::endl;
    std::ofstream("f1.json") << slices_json.dump(3) << std::endl;
}