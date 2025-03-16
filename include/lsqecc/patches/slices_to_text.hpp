#ifndef LSQECC_SLICES_TO_TEXT_HPP
#define LSQECC_SLICES_TO_TEXT_HPP

#include <lsqecc/patches/dense_slice.hpp>
#include <lsqecc/patches/sparse_slice.hpp>
#include <string>
#include <vector>

namespace lsqecc {

// Converts a single DenseSlice to text given a time stamp.
std::string slice_to_text(const DenseSlice& slice, size_t time_stamp);

// Converts a vector of DenseSlices to text.
std::string slices_to_text(const std::vector<DenseSlice>& slices);

} // namespace lsqecc

#endif //LSQECC_SLICES_TO_TEXT_HPP