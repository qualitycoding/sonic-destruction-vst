// FROZEN — DO NOT MODIFY
// T-004: exact continuous-phase zero-order-hold timing (D-005).
#include "dsp/SampleHold.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <vector>

using namespace faultcounter;
using Catch::Matchers::WithinAbs;

TEST_CASE ("T-004 SampleHold captures the first sample and is transparent at host rate", "[frozen][sample-hold]")
{
    SampleHold hold;
    hold.prepare (48000.0);
    hold.reset();
    REQUIRE_THAT (double (hold.process (0.25f, 48000.0)), WithinAbs (0.25, 1e-7));

    for (int i = 1; i < 64; ++i)
        REQUIRE_THAT (double (hold.process (float (i), 48000.0)), WithinAbs (double (i), 1e-6));
}

TEST_CASE ("T-004 SampleHold schedules 1 kHz and 280 Hz holds analytically", "[frozen][sample-hold]")
{
    constexpr double hostRate = 48000.0;
    constexpr int length = 200;
    std::vector<float> values (length);

    SampleHold hold;
    hold.prepare (hostRate);
    hold.reset();
    for (int i = 0; i < length; ++i)
        values[size_t (i)] = hold.process (float (i) / 1000.0f, 1000.0);

    for (int i = 0; i < 47; ++i)
        REQUIRE_THAT (double (values[size_t (i)]), WithinAbs (0.0, 1e-7));
    for (int i = 47; i < 95; ++i)
        REQUIRE_THAT (double (values[size_t (i)]), WithinAbs (0.047, 1e-7));
    REQUIRE_THAT (double (values[95]), WithinAbs (0.095, 1e-7));

    hold.reset();
    int captureCount = 0;
    for (int i = 0; i < 200; ++i)
    {
        const auto before = hold.heldValue();
        (void) hold.process (float (i) / 1000.0f, 280.0);
        if (hold.heldValue() != before)
            ++captureCount;
    }
    // D-005 captures at samples 0 and 171; the sample-0 capture stores 0.0f,
    // identical to the initial held value, so only the sample-171 capture is
    // observable as a value change (first-sample capture is proven by the
    // host-rate case above).
    REQUIRE (captureCount == 1);
}
