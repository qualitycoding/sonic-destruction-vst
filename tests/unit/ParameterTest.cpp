// T-001: parameter IDs, ranges, defaults, text formatting, and 00..FB table mapping (D-003).
#include "Parameters.h"
#include "PluginProcessor.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>

using namespace faultcounter;
using Catch::Matchers::WithinAbs;

namespace
{
const std::array<const char*, 10> expectedIds {{
    "gain", "tone", "toneEnabled", "sampleRate", "fine", "bitControl", "bitMode", "bitPosition", "table", "bypass"
}};
} // namespace

TEST_CASE ("T-001 parameter descriptors match D-003", "[frozen][parameters]")
{
    const auto& descriptors = parameterDescriptors();
    REQUIRE (descriptors.size() == 10);

    for (size_t i = 0; i < descriptors.size(); ++i)
    {
        CAPTURE (i);
        REQUIRE (descriptors[i].id == expectedIds[i]);
    }

    REQUIRE (descriptors[0].kind == ParameterKind::Float);
    REQUIRE (descriptors[1].kind == ParameterKind::Float);
    REQUIRE (descriptors[2].kind == ParameterKind::Bool);
    REQUIRE (descriptors[3].kind == ParameterKind::Float);
    REQUIRE (descriptors[4].kind == ParameterKind::Bool);
    REQUIRE (descriptors[5].kind == ParameterKind::Float);
    REQUIRE (descriptors[6].kind == ParameterKind::Choice);
    REQUIRE (descriptors[7].kind == ParameterKind::Choice);
    REQUIRE (descriptors[8].kind == ParameterKind::Int);
    REQUIRE (descriptors[9].kind == ParameterKind::Bool);

    REQUIRE (descriptors[0].minimum == 0.0f);
    REQUIRE (descriptors[0].maximum == 1.0f);
    REQUIRE (descriptors[0].defaultValue == 0.2f);

    REQUIRE (descriptors[3].minimum == 280.0f);
    REQUIRE (descriptors[3].maximum == 58000.0f);
    REQUIRE (descriptors[3].defaultValue == 58000.0f);

    REQUIRE (descriptors[5].defaultValue == 1.0f);
    REQUIRE (descriptors[6].choiceCount == 2);
    REQUIRE (descriptors[7].choiceCount == 2);
    REQUIRE (descriptors[8].minimum == 0.0f);
    REQUIRE (descriptors[8].maximum == 251.0f);
}

TEST_CASE ("T-001 text formatting matches D-003", "[frozen][parameters]")
{
    REQUIRE (tableDisplay (0) == "00");
    REQUIRE (tableDisplay (42) == "2A");
    REQUIRE (tableDisplay (251) == "FB");

    REQUIRE (bitDisplay (0.0f, false) == "1 bit");
    REQUIRE (bitDisplay (1.0f, false) == "8 bit");
    REQUIRE (bitDisplay (2.0f / 7.0f, false) == "3 bit");
    REQUIRE (bitDisplay (0.0f, true) == "000");
    REQUIRE (bitDisplay (1.0f, true) == "255");
    REQUIRE (bitDisplay (0.5f, true) == "128");

    REQUIRE (sampleRateDisplay (280.0f) == "280.0 Hz");
    REQUIRE (sampleRateDisplay (999.9f) == "999.9 Hz");
    REQUIRE (sampleRateDisplay (58000.0f) == "58.00 kHz");
    REQUIRE (sampleRateDisplay (9000.0f) == "9.00 kHz");
}

TEST_CASE ("T-001 numeric mappings match D-003", "[frozen][parameters]")
{
    REQUIRE_THAT (double (normalizedToSampleRate (0.0f)), WithinAbs (280.0, 1e-4));
    REQUIRE_THAT (double (normalizedToSampleRate (1.0f)), WithinAbs (58000.0, 1e-2));
    REQUIRE_THAT (double (normalizedToSampleRate (0.5f)), WithinAbs (double (280.0f * std::pow (58000.0f / 280.0f, 0.5f)), 1e-2));

    for (const float v : { 0.0f, 0.13f, 0.5f, 0.87f, 1.0f })
        REQUIRE_THAT (double (sampleRateToNormalized (normalizedToSampleRate (v))), WithinAbs (double (v), 1e-5));

    REQUIRE (bitControlToBits (0.0f) == 1);
    REQUIRE (bitControlToBits (1.0f) == 8);
    REQUIRE (bitControlToBits (2.0f / 7.0f) == 3);
    REQUIRE (bitControlToMask (0.0f) == 0);
    REQUIRE (bitControlToMask (1.0f) == 255);
    REQUIRE (bitControlToMask (0.5f) == 128);

    // Non-finite inputs fall back to the D-003 defaults (bitControl -> 1.0).
    REQUIRE (bitControlToBits (std::numeric_limits<float>::quiet_NaN()) == 8);
    REQUIRE (bitControlToMask (std::numeric_limits<float>::quiet_NaN()) == 255);
}

TEST_CASE ("T-001 APVTS exposes the D-003 schema", "[frozen][parameters]")
{
    FaultCounterProcessor processor;
    auto& state = processor.apvts();

    REQUIRE (processor.getParameters().size() == 10);
    for (const char* id : expectedIds)
        REQUIRE (state.getParameter (id) != nullptr);

    REQUIRE_THAT (double (state.getRawParameterValue ("gain")->load()), WithinAbs (0.2, 1e-6));
    REQUIRE_THAT (double (state.getRawParameterValue ("tone")->load()), WithinAbs (0.5, 1e-6));
    REQUIRE (state.getRawParameterValue ("toneEnabled")->load() == 1.0f);
    REQUIRE_THAT (double (state.getRawParameterValue ("sampleRate")->load()), WithinAbs (58000.0, 1e-1));
    REQUIRE (state.getRawParameterValue ("fine")->load() == 0.0f);
    REQUIRE_THAT (double (state.getRawParameterValue ("bitControl")->load()), WithinAbs (1.0, 1e-6));
    REQUIRE (state.getRawParameterValue ("bitMode")->load() == 0.0f);
    REQUIRE (state.getRawParameterValue ("bitPosition")->load() == 0.0f);
    REQUIRE (state.getRawParameterValue ("table")->load() == 0.0f);
    REQUIRE (state.getRawParameterValue ("bypass")->load() == 0.0f);
}
