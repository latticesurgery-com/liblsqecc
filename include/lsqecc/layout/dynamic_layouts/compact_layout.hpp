#ifndef LSQECC_COMPACT_LAYOUT_HPP
#define LSQECC_COMPACT_LAYOUT_HPP

#include <lsqecc/layout/ascii_layout_spec.hpp>



namespace lsqecc
{


/**
 * Allocates a layout like so:
 *
 * QQQQ ... QrAr11111
 * rrrr ... rrrr11111
 * QQQQ ... QrAr11111
 */

std::unique_ptr<Layout> make_compact_layout(size_t num_core_qubits);


} // namespace lsqecc

#endif //LSQECC_COMPACT_LAYOUT_HPP
