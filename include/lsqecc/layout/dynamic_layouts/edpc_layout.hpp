#ifndef LSQECC_EDPC_LAYOUT_HPP
#define LSQECC_EDPC_LAYOUT_HPP

#include <memory>
#include <lsqecc/layout/ascii_layout_spec.hpp>



namespace lsqecc
{


/**
 * Allocates a layout like so (distillation regions are placed along edges):
 *
 * rrrrrrr ... 11111
 * rQrQrQr ... 11111
 * rrrrrrr ... 11111
 * rQrQrQr ... 22222
 * rrrrrrr ... 22222
 * .
 * .
 * .
 * 
 */

std::unique_ptr<Layout> make_edpc_layout(size_t num_core_qubits);


} // namespace lsqecc

#endif //LSQECC_EDPC_LAYOUT_HPP


/* 
For 9 data qubits, this could look like
     222 555     
     222 555     
     222 555     
     222 555     
     222 555     
00000rrrrrrr88888
00000rQrQrQr88888
00000rrrrrrr88888
     rQrQrQr
11111rrrrrrr77777
11111rQrQrQr77777
11111rrrrrrr77777
     333 444     
     333 444     
     333 444     
     333 444     
     333 444    


For 16 data qubits, this could look like:
(assuming we are currently limited to 10 distillation factories)
(a problem, for now, is that the 1 and 7 distillation blocks are shifted)
00000rrrrrrrrr88888
00000rQrQrQrQr88888
00000rrrrrrrrr88888
11111rQrQrQrQr77777
11111rrrrrrrrr77777
11111rQrQrQrQr77777
22222rrrrrrrrr22222
22222rQrQrQrQr22222
22222rrrrrrrrr22222
     333444555     
     333444555     
     333444555     
     333444555               
     333444555     
     333444555 

We will want a way to automatically generate these and to also involve better distillation protocols.                                   
*/
