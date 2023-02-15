#ifndef LSQECC_EDPC_LAYOUT_HPP
#define LSQECC_EDPC_LAYOUT_HPP

#include <memory>
#include <lsqecc/layout/ascii_layout_spec.hpp>



namespace lsqecc
{

std::unique_ptr<Layout> make_edpc_layout(size_t num_core_qubits, const DistillationOptions& distillation_options);


} // namespace lsqecc

#endif //LSQECC_EDPC_LAYOUT_HPP


/* 
For 9 data qubits, this could look like
     222 555     
     222 555     
     222 555     
     222 555     
    ArMrArMrA     
0000rrrrrrrrr8888
0000MrQrQrQrM8888
0000rrrrrrrrr8888
    ArQrQrQrA
1111rrrrrrrrr7777
1111MrQrQrQrM7777
1111rrrrrrrrr7777
    ArMrArMrA
     333 444     
     333 444     
     333 444     
     333 444                                   
*/
