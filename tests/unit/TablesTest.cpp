// FROZEN — DO NOT MODIFY
// T-005: exact procedural asset dimensions, hash, and per-family row uniqueness (D-008).
#include "dsp/Tables.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <set>
#include <string>

using namespace faultcounter;

TEST_CASE ("T-005 procedural table asset is complete and deterministic", "[frozen][tables]")
{
    const auto path = std::filesystem::path (FAULTCOUNTER_TABLE_ASSET_PATH);
    const auto tables = Tables::loadProcedural (path);

    REQUIRE (std::filesystem::file_size (path) == 252 * 256);
    REQUIRE (tables.size() == 252);
    REQUIRE (tables.data().size() == 252 * 256);

    // SHA-256 of assets/tables/procedural.bin, generated from D-008.
    // If this changes, the generator amendment procedure—not this assertion—must be used.
    REQUIRE (path.filename() == "procedural.bin");
    REQUIRE (Tables::sha256File (path) == "226c57a74537e21f5c40dd9cac2eacbe1836f0777040eb86d820fa8863a35631");

    for (int family = 0; family < 9; ++family)
    {
        std::set<std::string> uniqueRows;
        for (int row = 0; row < 28; ++row)
        {
            std::string bytes;
            bytes.reserve (256);
            for (int code = 0; code < 256; ++code)
                bytes.push_back (char (tables.lookup (family * 28 + row, uint8_t (code))));
            uniqueRows.insert (bytes);
        }
        REQUIRE (uniqueRows.size() == 28);
    }

    REQUIRE (tables.lookup (0, 0) == 0);
    REQUIRE (tables.lookup (0, 255) == 255);
    REQUIRE (tables.lookup (251, 128) == 128); // asymmetric threshold zero at center
}
