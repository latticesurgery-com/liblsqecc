#ifndef LSQECC_EDPC_LAYOUT_HPP
#define LSQECC_EDPC_LAYOUT_HPP

#include <memory>
#include <lsqecc/layout/ascii_layout_spec.hpp>



namespace lsqecc
{

std::unique_ptr<Layout> make_edpc_layout(size_t num_core_qubits, size_t num_lanes, bool condensed, bool factories_explicit, bool predistilled, const DistillationOptions& distillation_options);


} // namespace lsqecc

#endif //LSQECC_EDPC_LAYOUT_HPP
