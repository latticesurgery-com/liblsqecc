#include <lsqecc/layout/layout.hpp>


namespace lsqecc {

const std::vector<Patch> SimpleLayout::core_patches() const
{
    std::vector<Patch> core;
    core.reserve(num_qubits_);
    for(size_t i = 0; i<num_qubits_; i++) {
        core.push_back(basic_square_patch({
                        .row=0,
                        .col=2*static_cast<Cell::CoordinateType>(i)
        }));
    }
    return core;
}



}