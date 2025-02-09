#ifndef LSQECC_SLICES_TO_JSON_HPP
#define LSQECC_SLICES_TO_JSON_HPP

#include <lsqecc/patches/dense_slice.hpp>
#include <lsqecc/patches/sparse_slice.hpp>
#include <nlohmann/json.hpp>

namespace lsqecc {

nlohmann::json slice_to_json(const SparseSlice& slices);
nlohmann::json slice_to_json(const DenseSlice& slices);


template<class SliceType>
nlohmann::json slices_to_json(const std::vector<SliceType>& slices)
{
    static_assert(std::is_same_v<SliceType, SparseSlice> || std::is_same_v<SliceType, DenseSlice>);

    nlohmann::json out_slices = nlohmann::json::array();

    for(const SparseSlice& slice : slices)
        out_slices.push_back(slice_to_json(slice));


    return out_slices;
}


}


#endif //LSQECC_SLICES_TO_JSON_HPP
