//
// Created by george on 2022-02-15.
//

#include <lsqecc/layout/parse_layout_spec.hpp>
#include <absl/strings/str_split.h>

static_assert(std::is_same_v<absl::string_view,std::string_view>);

namespace lsqecc{


    std::vector<std::vector<LayoutSpec::CellType>> LayoutSpec::from_string_view(std::string_view input)
    {
        std::vector<std::vector<LayoutSpec::CellType>> rows;
        for(const std::string_view& row: absl::StrSplit(input,'\n'))
        {
            rows.emplace_back(row.size());
            for(char c: row)
            {
                rows.back().push_back(cell_type_from_char(c));
            }
        }
        return rows;
    }



}
