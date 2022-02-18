#ifndef LSQECC_SLICES_TO_JSON_HPP
#define LSQECC_SLICES_TO_JSON_HPP

#include <lsqecc/patches/fast_patch_computation.hpp>
#include <nlohmann/json.hpp>

namespace lsqecc {


nlohmann::json cell_patch_to_visual_array_edges_json(const SingleCellOccupiedByPatch& boundary);

nlohmann::json computation_to_json(const PatchComputation& computation);

}


#endif //LSQECC_SLICES_TO_JSON_HPP
