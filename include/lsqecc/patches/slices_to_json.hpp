#ifndef LSQECC_SLICES_TO_JSON_HPP
#define LSQECC_SLICES_TO_JSON_HPP

#include <lsqecc/patches/slice.hpp>
#include <nlohmann/json.hpp>

namespace lsqecc {


nlohmann::json cell_patch_to_visual_array_edges_json(const SingleCellOccupiedByPatch& boundary);

nlohmann::json slices_to_json(const std::vector<Slice>& slices);

}


#endif //LSQECC_SLICES_TO_JSON_HPP
