#include "lsqecc/patches/minetest_export.hpp"
#include "lsqecc/patches/patches.hpp"
#include <sqlite3.h>
#include <iostream>
#include <array>
#include <cassert>
#include <cstring>
#include <string>
#include <string_view>
#include <cstdint>
#include <vector>
#include <zstd.h>

namespace lsqecc {

namespace {

// ---- Compile-time mapblock frame ----

constexpr std::string_view kMappingNames[] = {
    /* 0  */ "air",
    /* 1  */ "default:stone",
    /* 2  */ "default:stone_with_coal",
    /* 3  */ "default:obsidian",
    /* 4  */ "default:cobble",
    /* 5  */ "default:dirt",
    /* 6  */ "default:dirt_with_grass",
    /* 7  */ "default:dirt_with_rainforest_litter",
    /* 8  */ "default:dirt_with_dry_grass",
    /* 9  */ "default:dry_dirt",
    /* 10 */ "default:dry_dirt_with_dry_grass",
    /* 11 */ "default:silver_sand",
    /* 12 */ "default:gravel",
    /* 13 */ "default:glass",
    /* 14 */ "default:papyrus",
    /* 15 */ "default:cactus",
    /* 16 */ "default:snow",
    /* 17 */ "default:lava_source",
    /* 18 */ "default:lava_flowing",
    /* 19 */ "default:water_source",
    /* 20 */ "default:coral_pink",
    /* 21 */ "default:ice",
    /* 22 */ "default:permafrost",
    /* 23 */ "default:mossycobble",
};
constexpr size_t kNumMappings = std::size(kMappingNames);

// Material IDs are indices into kMappingNames above.
constexpr uint16_t kMatAir       = 0;  // empty space
constexpr uint16_t kMatDirt      = 5;  // qubit
constexpr uint16_t kMatGlass     = 13; // route (empty cell)
constexpr uint16_t kMatCactus    = 15; // ancilla / routing / dead
constexpr uint16_t kMatCoralPink = 20; // distillation / prepared state
constexpr uint16_t kMatIce       = 21; // qubit under measurement

constexpr size_t kHeaderSize = []() constexpr {
    size_t s = 10; // 7 flags/ts + 1 version + 2 num_mappings
    for (const auto& n : kMappingNames) s += 4 + n.size();
    return s + 2; // content_width + params_width
}();

constexpr std::array<char, kHeaderSize> MakeHeader()
{
    std::array<char, kHeaderSize> h{};
    size_t i = 8; // bytes 0-6: zero flags/ts, byte 7: zero version
    h[i++] = static_cast<char>((kNumMappings >> 8) & 0xFF);
    h[i++] = static_cast<char>(kNumMappings & 0xFF);
    for (size_t mi = 0; mi < kNumMappings; ++mi) {
        const auto& nm = kMappingNames[mi];
        // id high byte is 0 (ids 0-23 fit in one byte, array already zeroed)
        h[i++] = '\x00';
        h[i++] = static_cast<char>(mi & 0xFF);
        const auto nl = static_cast<uint16_t>(nm.size());
        h[i++] = static_cast<char>((nl >> 8) & 0xFF);
        h[i++] = static_cast<char>(nl & 0xFF);
        for (char c : nm) h[i++] = c;
    }
    h[i++] = '\x02'; // content_width
    h[i++] = '\x02'; // params_width
    return h;
}

// param1 (4096 zeros) + param2 (4096 zeros) + trailing (\0\0\0\0\n\0\0)
constexpr size_t kSuffixSize = 4096 + 4096 + 7;
constexpr std::array<char, kSuffixSize> MakeSuffix()
{
    std::array<char, kSuffixSize> s{};
    s[4096 + 4096 + 4] = '\n';
    return s;
}

constexpr auto kHeader = MakeHeader();
constexpr auto kSuffix = MakeSuffix();

constexpr size_t kParam0Size = 16 * 16 * 16 * 2;
constexpr size_t kBlobSize   = kHeader.size() + kParam0Size + kSuffix.size();
constexpr uint8_t kVersion   = 29;

// ---- Helpers ----

uint16_t PatchToMaterialId(const std::optional<DensePatch>& patch)
{
    if (!patch.has_value())
        return kMatGlass;
    switch (patch->type) {
    case PatchType::Qubit:
        return (patch->activity == PatchActivity::Measurement) ? kMatIce : kMatDirt;
    case PatchType::Routing:
    case PatchType::Dead:
        return kMatCactus;
    case PatchType::Distillation:
    case PatchType::PreparedState:
        return kMatCoralPink;
    default:
        return kMatAir;
    }
}

// Packs a Minetest block position into its database key. Valid only for non-negative
// block coordinates, which always holds here: row, y (= time_stamp * stripe_height)
// and col are all >= 0.
int64_t ComputeMapblockPosition(int mb_x, int mb_y, int mb_z)
{
    assert(mb_x >= 0 && mb_y >= 0 && mb_z >= 0);
    return (static_cast<int64_t>(mb_z) << 24) | (static_cast<int64_t>(mb_y) << 12) | mb_x;
}

} // namespace

// ---- Constructor / destructor ----

MinetestMapBuilder::MinetestMapBuilder(const std::string& db_path)
{
    if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db_) << "\n";
        ok_ = false;
        return;
    }

    sqlite3_exec(db_, "PRAGMA journal_mode=WAL;",    nullptr, nullptr, nullptr);
    sqlite3_exec(db_, "PRAGMA synchronous=NORMAL;",  nullptr, nullptr, nullptr);

    char* errmsg = nullptr;
    if (sqlite3_exec(db_, "CREATE TABLE IF NOT EXISTS blocks (pos INTEGER PRIMARY KEY, data BLOB);",
                     nullptr, nullptr, &errmsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errmsg << "\n";
        sqlite3_free(errmsg);
        ok_ = false;
        return;
    }

    if (sqlite3_prepare_v2(db_, "INSERT OR REPLACE INTO blocks (pos, data) VALUES (?, ?);",
                           -1, &stmt_, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << "\n";
        ok_ = false;
        return;
    }

    // Pre-allocate the reusable mapblock blob: header | param0 | suffix.
    blob_.assign(kBlobSize, '\0');
    std::memcpy(blob_.data(), kHeader.data(), kHeader.size());
    std::memcpy(blob_.data() + kHeader.size() + kParam0Size, kSuffix.data(), kSuffix.size());

    // Pre-allocate the compression output buffer.
    compress_buf_.resize(1 + ZSTD_compressBound(kBlobSize));

    sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
}

MinetestMapBuilder::~MinetestMapBuilder()
{
    if (db_) {
        // finish() was not called; roll back any uncommitted changes.
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_finalize(stmt_);
        sqlite3_close(db_);
    }
}

// ---- Slice accumulation ----

void MinetestMapBuilder::add_slice(const DenseSlice& s, size_t time_stamp, int stripe_height)
{
    if (!ok_) return;

    const int y      = static_cast<int>(time_stamp) * stripe_height;
    const int mb_y   = y / 16;
    const int inner_y = y % 16;

    // When we move to a new y-band, everything in blocks_ belongs to the completed
    // previous band and can be flushed immediately.
    if (mb_y != current_mb_y_) {
        if (current_mb_y_ != -1)
            FlushCurrentBand();
        current_mb_y_ = mb_y;
    }

    const Cell furthest = s.get_layout().furthest_cell();
    for (int row = 0; row <= furthest.row; ++row) {
        for (int col = 0; col <= furthest.col; ++col) {
            const uint16_t mat = PatchToMaterialId(s.patch_at(Cell::from_ints(row, col)));
            if (mat == kMatAir)
                continue; // don't materialize all-air blocks
            const int64_t pos   = ComputeMapblockPosition(row / 16, mb_y, col / 16);
            const int     inner = (col % 16) * 256 + inner_y * 16 + (row % 16);
            blocks_[pos][static_cast<size_t>(inner)] = mat;
        }
    }
}

// ---- Internal flush ----

void MinetestMapBuilder::FlushCurrentBand()
{
    char* const param0_ptr = blob_.data() + kHeader.size();

    for (const auto& [pos, nodes] : blocks_) {
        if (!ok_) break;

        for (int i = 0; i < 4096; ++i) {
            const uint16_t v = nodes[static_cast<size_t>(i)];
            param0_ptr[2 * i]     = static_cast<char>((v >> 8) & 0xFF);
            param0_ptr[2 * i + 1] = static_cast<char>(v & 0xFF);
        }

        size_t compressed_size = ZSTD_compress(
            compress_buf_.data() + 1, compress_buf_.size() - 1,
            blob_.data(), blob_.size(), 3);
        if (ZSTD_isError(compressed_size)) {
            std::cerr << "Compression error: " << ZSTD_getErrorName(compressed_size) << "\n";
            ok_ = false;
            break;
        }
        compress_buf_[0] = static_cast<char>(kVersion);

        sqlite3_reset(stmt_);
        sqlite3_bind_int64(stmt_, 1, pos);
        sqlite3_bind_blob(stmt_, 2, compress_buf_.data(),
                          static_cast<int>(1 + compressed_size), SQLITE_STATIC);
        if (sqlite3_step(stmt_) != SQLITE_DONE) {
            std::cerr << "Insert failed for block " << pos
                      << ": " << sqlite3_errmsg(db_) << "\n";
            ok_ = false;
            break;
        }
        ++total_blocks_written_;
    }
    blocks_.clear();
}

// ---- Finish ----

bool MinetestMapBuilder::finish()
{
    if (!db_) return false; // already finished or never opened

    if (ok_)
        FlushCurrentBand(); // flush the last band

    sqlite3_exec(db_, ok_ ? "COMMIT;" : "ROLLBACK;", nullptr, nullptr, nullptr);
    sqlite3_finalize(stmt_); stmt_ = nullptr;
    sqlite3_close(db_);      db_   = nullptr;
    return ok_;
}

} // namespace lsqecc
