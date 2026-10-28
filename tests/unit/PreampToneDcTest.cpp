// FROZEN — DO NOT MODIFY
// T-007, T-008: fallback preamp/tone boundary behavior and DC-blocker stability (D-006/D-009).
#include "dsp/DcBlocker.h"
#include "dsp/Preamp.h"
#include "dsp/Tone.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>
#include <vector>

using namespace faultcounter;
using Catch::Matchers::WithinAbs;

TEST_CASE ("T-007 zero gain with tone disabled is linear", "[frozen][preamp][tone]")
{
    Preamp preamp;
    preamp.setGain (0.0f);
    REQUIRE_THAT (double (preamp.process (-0.375f)), WithinAbs (-0.375, 1e-6));
    REQUIRE_THAT (double (preamp.process (0.875f)), WithinAbs (0.875, 1e-6));

    Tone tone;
    tone.prepare (48000.0);
    tone.setTone (0.25f, false);
    REQUIRE_THAT (double (tone.process (-0.375f)), WithinAbs (-0.375, 1e-6));

    tone.setTone (0.5f, true);
    REQUIRE_THAT (double (tone.process (-0.375f)), WithinAbs (-0.375, 1e-6));
}

TEST_CASE ("T-007 maximum gain remains finite and bounded", "[frozen][preamp]")
{
    Preamp preamp;
    preamp.setGain (1.0f);
    for (const float input : { -1.0f, -0.5f, 0.0f, 0.5f, 1.0f })
    {
        const float output = preamp.process (input);
        REQUIRE (std::isfinite (output));
        REQUIRE (std::abs (output) <= 1.5f);
    }
}

TEST_CASE ("T-008 DC blocker rejects a constant below -80 dBFS", "[frozen][dc-blocker]")
{
    constexpr double sampleRate = 48000.0;
    constexpr int length = int (sampleRate * 10.0);
    DcBlocker blocker;
    blocker.prepare (sampleRate);
    blocker.reset();

    double sumSquares = 0.0;
    for (int i = 0; i < length; ++i)
    {
        const float output = blocker.process (1.0f);
        REQUIRE (std::isfinite (output));
        if (i >= length / 2)
            sumSquares += double (output) * double (output);
    }

    const double rms = std::sqrt (sumSquares / (length / 2));
    REQUIRE (rms < std::pow (10.0, -80.0 / 20.0));
}
