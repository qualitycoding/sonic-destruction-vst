// FROZEN — DO NOT MODIFY
// T-006: pre/post routing order is proven with reversible and non-reversible test tables (D-007).
#include "dsp/Engine.h"
#include "dsp/BitOps.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <vector>

using namespace faultcounter;
using Catch::Matchers::WithinAbs;

namespace
{
std::vector<uint8_t> makeLowNibbleTable()
{
    std::vector<uint8_t> table (256);
    for (int code = 0; code < 256; ++code)
        table[size_t (code)] = uint8_t (code & 0x0f);
    return table;
}

std::vector<uint8_t> makeIdentityTable()
{
    std::vector<uint8_t> table (256);
    for (int code = 0; code < 256; ++code)
        table[size_t (code)] = uint8_t (code);
    return table;
}

float renderOne (Engine& engine, float input)
{
    float* channel = &input;
    float* channels[] = { channel };
    engine.process (channels, 1, 1);
    return input;
}
} // namespace

TEST_CASE ("T-006 identity table exposes Bits routing directly", "[frozen][engine]")
{
    Engine engine;
    engine.prepare (48000.0, makeIdentityTable());
    EngineSettings settings;
    settings.gain = 0.0f;
    settings.toneEnabled = false;
    settings.sampleRateHz = 58000.0f;
    settings.bitControl = 2.0f / 7.0f; // Bits 3
    settings.bitPosition = BitPosition::Pre;

    const float input = fromCode (0xac);
    settings.bitPosition = BitPosition::Pre;
    engine.setSettings (settings);
    REQUIRE_THAT (double (renderOne (engine, input)), WithinAbs (double (fromCode (0xb6)), 1e-6));

    engine.reset();
    settings.bitPosition = BitPosition::Post;
    engine.setSettings (settings);
    REQUIRE_THAT (double (renderOne (engine, input)), WithinAbs (double (fromCode (0xb6)), 1e-6));
}

TEST_CASE ("T-006 non-reversible table proves pre and post routes differ", "[frozen][engine]")
{
    Engine engine;
    engine.prepare (48000.0, makeLowNibbleTable());
    EngineSettings settings;
    settings.gain = 0.0f;
    settings.toneEnabled = false;
    settings.sampleRateHz = 58000.0f;
    settings.bitControl = 2.0f / 7.0f; // Bits 3

    const float input = fromCode (0xac);

    settings.bitPosition = BitPosition::Pre;
    engine.setSettings (settings);
    REQUIRE_THAT (double (renderOne (engine, input)), WithinAbs (double (fromCode (0x06)), 1e-6));

    engine.reset();
    settings.bitPosition = BitPosition::Post;
    engine.setSettings (settings);
    REQUIRE_THAT (double (renderOne (engine, input)), WithinAbs (double (fromCode (0x00)), 1e-6));
}
