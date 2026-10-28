// FROZEN — DO NOT MODIFY
// T-009: the complete engine matches independent golden PCM fixtures for impulse,
// ramp, sine, deterministic noise seed 0x47C0, and silence.
#include "dsp/Engine.h"
#include "dsp/Tables.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using namespace faultcounter;
using Catch::Matchers::WithinAbs;

namespace
{
constexpr int length = 512;
constexpr double sampleRate = 48000.0;

std::vector<float> loadGolden (const std::string& name)
{
    const auto path = std::filesystem::path (FAULTCOUNTER_TABLE_ASSET_PATH).parent_path().parent_path().parent_path()
                      / "tests/golden" / ("expected-" + name + ".f32");
    std::ifstream input (path, std::ios::binary);
    REQUIRE (input.good());
    std::vector<float> values (length);
    input.read (reinterpret_cast<char*> (values.data()), std::streamsize (values.size() * sizeof (float)));
    REQUIRE (input.gcount() == std::streamsize (values.size() * sizeof (float)));
    return values;
}

std::vector<float> render (const std::vector<float>& input, const std::vector<uint8_t>& tables)
{
    Engine engine;
    engine.prepare (sampleRate, tables);
    EngineSettings settings;
    settings.gain = 0.0f;
    settings.toneEnabled = false;
    settings.sampleRateHz = 9000.0f;
    settings.bitControl = 2.0f / 7.0f;
    settings.bitMode = BitMode::Bits;
    settings.bitPosition = BitPosition::Post;
    settings.table = 42;
    engine.setSettings (settings);

    auto copy = input;
    float* channels[] = { copy.data() };
    engine.process (channels, 1, length);
    return copy;
}

std::vector<float> makeInput (const std::string& name)
{
    std::vector<float> values (length, 0.0f);
    if (name == "impulse")
        values[0] = 1.0f;
    else if (name == "ramp")
        for (int i = 0; i < length; ++i)
            values[size_t (i)] = float (-1.0 + 2.0 * double (i) / double (length - 1));
    else if (name == "sine")
        for (int i = 0; i < length; ++i)
            values[size_t (i)] = float (0.5 * std::sin (2.0 * 3.14159265358979323846 * 997.0 * double (i) / sampleRate));
    else if (name == "noise-seed-0x47c0")
    {
        uint32_t state = 0x47C0;
        for (auto& value : values)
        {
            state = state * 1664525u + 1013904223u;
            value = float (double (state) / double (0x80000000u) - 1.0);
        }
    }
    return values;
}
} // namespace

TEST_CASE ("T-009 complete engine matches committed golden fixtures", "[frozen][engine][golden]")
{
    const auto tables = Tables::loadProcedural (FAULTCOUNTER_TABLE_ASSET_PATH);
    for (const auto& name : { "impulse", "ramp", "sine", "noise-seed-0x47c0", "silence" })
    {
        const auto expected = loadGolden (name);
        const auto actual = render (makeInput (name), tables.data());
        REQUIRE (actual.size() == expected.size());
        for (int i = 0; i < length; ++i)
        {
            CAPTURE (name, i, expected[size_t (i)], actual[size_t (i)]);
            REQUIRE_THAT (double (actual[size_t (i)]), WithinAbs (double (expected[size_t (i)]), 2e-6));
        }
    }
}
