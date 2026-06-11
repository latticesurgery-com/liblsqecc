#ifndef LSQECC_MINETEST_EXPORT_HPP
#define LSQECC_MINETEST_EXPORT_HPP

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <lsqecc/patches/dense_slice.hpp>

// Forward-declare sqlite3 to avoid including sqlite3.h in the public header.
struct sqlite3;
struct sqlite3_stmt;

namespace lsqecc {

// Accumulates DenseSlices into Minetest mapblock format and streams them to a
// map.sqlite database as each y-band is completed. Memory is bounded to one
// 16-node-tall band at a time regardless of circuit length.
//
// Usage:
//   MinetestMapBuilder builder("map.sqlite");
//   for each slice: builder.add_slice(s, t, stripe_height);
//   bool ok = builder.finish();
//
// If finish() is not called (e.g. on exception), the destructor rolls back.
class MinetestMapBuilder {
public:
#if defined(__EMSCRIPTEN__)
    explicit MinetestMapBuilder(const std::string&) {}
    ~MinetestMapBuilder() = default;
    void add_slice(const DenseSlice&, size_t, int) {}
    bool finish() { return false; }
    size_t block_count() const { return 0; }
#else
    explicit MinetestMapBuilder(const std::string& db_path);
    ~MinetestMapBuilder();

    // Non-copyable, non-movable (owns a raw DB handle).
    MinetestMapBuilder(const MinetestMapBuilder&) = delete;
    MinetestMapBuilder& operator=(const MinetestMapBuilder&) = delete;

    // Records one time slice. Flushes the previous y-band to the DB when mb_y advances.
    void add_slice(const DenseSlice& s, size_t time_stamp, int stripe_height);

    // Flushes the last band, commits the transaction, and closes the DB.
    // Returns false if any write failed. Must be called at most once.
    bool finish();

    // Total mapblocks accumulated (flushed to DB + pending in current band).
    size_t block_count() const { return total_blocks_written_ + blocks_.size(); }
#endif

private:
    sqlite3*      db_   = nullptr;
    sqlite3_stmt* stmt_ = nullptr;
    std::string        blob_;
    std::vector<char>  compress_buf_;
    std::unordered_map<int64_t, std::array<uint16_t, 4096>> blocks_;
    int    current_mb_y_        = -1;
    size_t total_blocks_written_ = 0;
    bool   ok_                   = true;

    // Writes every block in blocks_ to the DB, then clears blocks_.
    void FlushCurrentBand();
};

} // namespace lsqecc

#endif // LSQECC_MINETEST_EXPORT_HPP
