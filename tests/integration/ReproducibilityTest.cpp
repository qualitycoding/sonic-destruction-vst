// T-016: same-build renders are bit-identical, and block partitioning does
// not alter output outside the documented parameter-smoothing transition.
#include "PluginProcessor.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <vector>

using namespace faultcounter;

namespace
{
constexpr double hostRate = 48000.0;
constexpr int totalSamples = 16384;
// Settle window covers the block-granularity smoothing ramps (5 ms gain/tone,
// 1 ms sample rate at 48 kHz) with wide margin; per T-016 the partition may
// differ only inside that documented transition.
constexpr int settleSamples = 4096;

std::vector<float> renderPartitioned (const std::vector<int>& schedule)
{
    FaultCounterProcessor processor;
    auto& state = processor.apvts();
    state.getRawParameterValue ("gain")->store (0.6f);
    state.getRawParameterValue ("tone")->store (0.4f);
    state.getRawParameterValue ("toneEnabled")->store (1.0f);
    state.getRawParameterValue ("sampleRate")->store (12000.0f);
    state.getRawParameterValue ("fine")->store (0.0f);
    state.getRawParameterValue ("bitControl")->store (0.75f);
    state.getRawParameterValue ("bitMode")->store (1.0f);
    state.getRawParameterValue ("bitPosition")->store (0.0f);
    state.getRawParameterValue ("table")->store (107.0f);
    state.getRawParameterValue ("bypass")->store (0.0f);
    processor.prepareToPlay (hostRate, 4096);

    std::vector<float> output;
    output.reserve (size_t (totalSamples));
    int sampleOffset = 0;
    juce::MidiBuffer midi;
    for (const int blockSize : schedule)
    {
        juce::AudioBuffer<float> buffer (1, blockSize);
        auto* data = buffer.getWritePointer (0);
        for (int i = 0; i < blockSize; ++i, ++sampleOffset)
            data[i] = float (0.8 * std::sin (2.0 * 3.14159265358979323846 * 331.0 * double (sampleOffset) / hostRate)
                             + 0.1 * std::sin (2.0 * 3.14159265358979323846 * 9097.0 * double (sampleOffset) / hostRate));
        processor.processBlock (buffer, midi);
        output.insert (output.end(), data, data + blockSize);
    }
    return output;
}
} // namespace

TEST_CASE ("T-016 two same-build renders are bit-identical", "[frozen][reproducibility]")
{
    const std::vector<int> schedule (size_t (totalSamples) / 512, 512);
    const auto first = renderPartitioned (schedule);
    const auto second = renderPartitioned (schedule);
    REQUIRE (first.size() == second.size());
    for (size_t i = 0; i < first.size(); ++i)
    {
        CAPTURE (i);
        REQUIRE (first[i] == second[i]);
    }
}

TEST_CASE ("T-016 block partitioning does not alter output after settling", "[frozen][reproducibility]")
{
    const std::vector<int> uniform (size_t (totalSamples) / 512, 512);
    // Irregular schedule summing to the same total length.
    const std::vector<int> irregular = { 64, 1, 511, 2, 1024, 7, 4096, 16, 10000, 663 };
    const auto base = renderPartitioned (uniform);
    const auto varied = renderPartitioned (irregular);
    REQUIRE (base.size() == size_t (totalSamples));
    REQUIRE (varied.size() == base.size());
    for (size_t i = size_t (settleSamples); i < base.size(); ++i)
    {
        CAPTURE (i);
        REQUIRE (varied[i] == base[i]);
    }
}
