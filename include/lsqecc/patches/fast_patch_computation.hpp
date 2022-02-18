#ifndef LSQECC_FAST_PATCH_COMPUTATION_HPP
#define LSQECC_FAST_PATCH_COMPUTATION_HPP


#include <lsqecc/logical_lattice_ops/logical_lattice_ops.hpp>
#include <lsqecc/patches/slice.hpp>
#include <lsqecc/patches/patches.hpp>
#include <lsqecc/layout/layout.hpp>


namespace lsqecc {


class PatchComputation
{
public:

    // TODO turn to constructor
    static PatchComputation make(const LogicalLatticeComputation& logical_computation);

private:

    Slice& new_slice();

    std::unique_ptr<Layout> layout = nullptr;
    std::vector<Slice> slices;


public:
    const std::vector<Slice>& get_slices()const {return slices;}
    const Layout& get_layout() const {return *layout;}

};


}

#endif //LSQECC_FAST_PATCH_COMPUTATION_HPP
