#ifndef LSQECC_MINETEST_EXPORT_HPP
#define LSQECC_MINETEST_EXPORT_HPP

#include <string>

namespace lsqecc {

#if defined(__EMSCRIPTEN__)
// Provide a stub for Emscripten builds.
inline bool generate_minetest_map(const std::string& pattern_str,
                                  const std::string& db_path,
                                  int stripe_height) {
    // Map generation is not supported for Emscripten builds.
    return false;
}
#else
// Full declaration for native builds.
bool generate_minetest_map(const std::string& pattern_str,
                           const std::string& db_path,
                           int stripe_height);
#endif

} // namespace lsqecc

#endif // LSQECC_MINETEST_EXPORT_HPP