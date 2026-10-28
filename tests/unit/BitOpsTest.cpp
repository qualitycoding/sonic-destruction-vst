// FROZEN — DO NOT MODIFY
// T-002, T-003: exact unsigned 8-bit conversion and reduction conventions (D-004).
#include "dsp/BitOps.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <array>
#include <cmath>
#include <cstdint>

using namespace faultcounter;
using Catch::Matchers::WithinAbs;

TEST_CASE ("T-002 toCode/fromCode boundaries and bounds", "[frozen][bitops]")
{
    REQUIRE (toCode (-1.0f) == 0);
    REQUIRE (toCode (1.0f) == 255);
    REQUIRE (toCode (0.0f) == 128);
    REQUIRE (toCode (-0.25f) == 96);
    REQUIRE (toCode (-100.0f) == 0);
    REQUIRE (toCode (100.0f) == 255);
    REQUIRE (! std::isnan (fromCode (0)));
    REQUIRE (! std::isnan (fromCode (255)));
    REQUIRE_THAT (double (fromCode (127)), WithinAbs (-1.0 / 255.0, 1e-7));
    REQUIRE_THAT (double (fromCode (128)), WithinAbs (1.0 / 255.0, 1e-7));
}

TEST_CASE ("T-003 Bits and Mask golden vectors", "[frozen][bitops]")
{
    const std::array<std::pair<int, uint8_t>, 8> expected {{
        { 1, 0xff }, { 2, 0xaa }, { 3, 0xb6 }, { 4, 0xaa },
        { 5, 0xad }, { 6, 0xaa }, { 7, 0xad }, { 8, 0xac }
    }};

    for (auto [bits, expectedCode] : expected)
        REQUIRE (applyBits (0xac, bits) == expectedCode);

    REQUIRE (applyBits (0, 1) == 0);
    REQUIRE (applyBits (0xff, 1) == 0xff);
    REQUIRE (applyBits (1, 8) == 1);
    REQUIRE (applyMask (0xac, 0xf0) == 0xa0);
    REQUIRE (applyMask (0xac, 0xff) == 0xac);
    REQUIRE (applyMask (0xac, 0x00) == 0x00);
}
