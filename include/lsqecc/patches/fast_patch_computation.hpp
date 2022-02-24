#ifndef LSQECC_FAST_PATCH_COMPUTATION_HPP
#define LSQECC_FAST_PATCH_COMPUTATION_HPP


#include <lsqecc/logical_lattice_ops/logical_lattice_ops.hpp>
#include <lsqecc/patches/slice.hpp>
#include <lsqecc/patches/patches.hpp>
#include <lsqecc/layout/layout.hpp>


#include <chrono>

namespace lsqecc {

class PatchComputation
{
public:

    PatchComputation (
            const LogicalLatticeComputation& logical_computation,
            std::unique_ptr<Layout>&& layout,
            std::optional<std::chrono::seconds> timeout);

private:

    void make_slices(const LogicalLatticeComputation& logical_computation, std::optional<std::chrono::seconds> timeout);

    Slice& new_slice();
    Slice& last_slice();

    std::unique_ptr<Layout> layout_ = nullptr;
    std::vector<Slice> slices_;

public:
    const std::vector<Slice>& get_slices()const {return slices_;}
    const Layout& get_layout() const {return *layout_;}

};


}

#endif //LSQECC_FAST_PATCH_COMPUTATION_HPP
