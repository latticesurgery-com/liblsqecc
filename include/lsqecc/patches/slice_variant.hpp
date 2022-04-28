#ifndef LSQECC_SLICE_VARIANT_HPP
#define LSQECC_SLICE_VARIANT_HPP

#include <lsqecc/patches/dense_slice.hpp>
#include <lsqecc/patches/sparse_slice.hpp>


namespace lsqecc
{

using SliceVariant = std::variant<DenseSlice, SparseSlice>;

}

#endif //LSQECC_SLICE_VARIANT_HPP
