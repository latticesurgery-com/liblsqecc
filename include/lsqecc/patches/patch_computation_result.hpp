#ifndef LSQECC_PATCH_COMPUTATION_RESULT_HPP
#define LSQECC_PATCH_COMPUTATION_RESULT_HPP

#include <cstddef>

namespace lsqecc {

struct PatchComputationResult {
    virtual size_t ls_instructions_count() const = 0;
    virtual size_t slice_count() const = 0;

    virtual ~PatchComputationResult(){};
};

}

#endif //LSQECC_PATCH_COMPUTATION_RESULT_HPP
