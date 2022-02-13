
#include <lsqecc/logical_lattice_ops/ls_instructions_parse.hpp>
#include <lsqecc/logical_lattice_ops/parse_utils.hpp>

#include <ranges>
#include <stdexcept>

namespace lsqecc {

tsl::ordered_map<PatchId, PauliOperator> parse_multi_body_measurement_dict(std::string_view dict_arg)
{
    auto dict_pairs = sv_split_on_char(dict_arg,',');
    tsl::ordered_map<PatchId, PauliOperator> ret;

    for(auto pair : dict_pairs){
        auto assoc = sv_split_on_char(pair,':');
        if(assoc.size() != 2) {
            throw std::logic_error(std::string{"MultiBody dict_pairs not in key pair format:"}+std::string{pair});
        }
        ret.insert_or_assign(parse_patch_id(assoc[0]), PauliOperator_from_string(assoc[1]));
    }
    return ret;
}


LogicalLatticeOperation parse_ls_instruction(std::string_view line)
{
    auto args = sv_split_on_char(line,' ');
    auto args_itr = args.begin();

    auto has_next_arg = [&](){return args_itr<args.end();};
    auto get_next_arg = [&](){
        if(!has_next_arg())
            throw std::out_of_range{"Out of arguments"};
        return *args_itr++;
    };

    std::string_view instruction = get_next_arg();

    if(instruction == "MeasureSinglePatch")
    {
        auto op = PauliOperator_from_string(get_next_arg());
        auto patch_id = parse_patch_id(get_next_arg());
        return LogicalLatticeOperation{SinglePatchMeasurement{patch_id,op,false}};
    }
    else if (instruction == "MultiBodyMeasure")
    {
        auto patches_dict = get_next_arg();
        return LogicalLatticeOperation{MultiPatchMeasurement{parse_multi_body_measurement_dict(patches_dict)}};
    }
    else if(instruction == "RequestMagicState")
    {
        auto patch_id = parse_patch_id(get_next_arg());
        return LogicalLatticeOperation{MagicStateRequest{patch_id}};
    }
    else if (instruction == "LogicalPauli")
    {
        auto op = PauliOperator_from_string(get_next_arg());
        auto patch_id = parse_patch_id(get_next_arg());
        return LogicalLatticeOperation{LogicalPauli{patch_id,op}};
    }
    else
    {
        throw std::logic_error(std::string{"Operation not supported: "}+std::string{instruction});
    }
}

std::vector<LogicalLatticeOperation> parse_ls_instructions(std::string_view source)
{
    std::vector<LogicalLatticeOperation> ret;

    auto view = sv_split_on_char(source,'\n');

    for (auto line : view){
        if(line != "")
        {
            ret.push_back(parse_ls_instruction(line));
        }

    }

    return ret;

}
}

