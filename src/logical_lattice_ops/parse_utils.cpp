#include <lsqecc/logical_lattice_ops/parse_utils.hpp>
#include <lsqecc/patches/patches.hpp>

#include <vector>
#include <ranges>
#include <charconv>
#include <absl/strings/str_split.h>

namespace lsqecc
{

std::vector<std::string_view> sv_split_on_char(std::string_view source, char c)
{
    return absl::StrSplit(source, c);
}

PatchId parse_patch_id(const std::string_view & input)
{
    int out;
    const std::from_chars_result result = std::from_chars(input.data(), input.data() + input.size(), out);
    if(result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range)
    {
        throw std::runtime_error(std::string{"Cannot parse_grid patch id "}+std::string{input});
    }
    return out;
}

}