// T-014: seeded malformed state/parameter fuzz under the processor — no
// crash, no NaN/Inf escape. Case count is fixed-seed and reproducible; CI
// overrides it to 100,000 via FAULTCOUNTER_FUZZ_CASES.
#include "PluginProcessor.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <vector>

using namespace faultcounter;

namespace
{
// SplitMix32: tiny deterministic PRNG; the seed is pinned so every run and
// every CI job exercises the identical case sequence.
class Rng
{
public:
    explicit Rng (uint32_t seed) : state_ (seed) {}

    uint32_t next()
    {
        state_ += 0x9e3779b9u;
        uint32_t z = state_;
        z = (z ^ (z >> 16)) * 0x21f0aaadu;
        z = (z ^ (z >> 15)) * 0x735a2d97u;
        return z ^ (z >> 16);
    }

    float unit() { return float (next() >> 8) / float (1 << 24); }
    int range (int lo, int hi) { return lo + int (next() % uint32_t (hi - lo + 1)); }

private:
    uint32_t state_;
};

int fuzzCaseCount()
{
    if (const char* env = std::getenv ("FAULTCOUNTER_FUZZ_CASES"))
        return std::max (1, std::atoi (env));
    return 2000;
}

void randomiseParameters (FaultCounterProcessor& processor, Rng& rng)
{
    auto& state = processor.apvts();
    state.getRawParameterValue ("gain")->store (rng.unit());
    state.getRawParameterValue ("tone")->store (rng.unit());
    state.getRawParameterValue ("toneEnabled")->store (rng.unit() < 0.5f ? 1.0f : 0.0f);
    state.getRawParameterValue ("sampleRate")->store (rng.range (280, 58000));
    state.getRawParameterValue ("fine")->store (rng.unit() < 0.5f ? 1.0f : 0.0f);
    state.getRawParameterValue ("bitControl")->store (rng.unit());
    state.getRawParameterValue ("bitMode")->store (rng.unit() < 0.5f ? 1.0f : 0.0f);
    state.getRawParameterValue ("bitPosition")->store (rng.unit() < 0.5f ? 1.0f : 0.0f);
    state.getRawParameterValue ("table")->store (float (rng.range (0, 251)));
    state.getRawParameterValue ("bypass")->store (rng.unit() < 0.2f ? 1.0f : 0.0f);
}

void randomiseStateChunk (FaultCounterProcessor& processor, Rng& rng)
{
    juce::MemoryBlock chunk;
    const int mode = rng.range (0, 3);
    if (mode == 0)
    {
        // Pure garbage of random length.
        const size_t length = size_t (rng.range (0, 2048));
        chunk.setSize (length);
        auto* bytes = static_cast<uint8_t*> (chunk.getData());
        for (size_t i = 0; i < length; ++i)
            bytes[i] = uint8_t (rng.next());
    }
    else if (mode == 1)
    {
        // Valid chunk with random byte mutations.
        processor.getStateInformation (chunk);
        auto* bytes = static_cast<uint8_t*> (chunk.getData());
        const int mutations = rng.range (1, 32);
        for (int i = 0; i < mutations && chunk.getSize() > 0; ++i)
            bytes[size_t (rng.next() % uint32_t (chunk.getSize()))] = uint8_t (rng.next());
    }
    else if (mode == 2)
    {
        // Truncated valid chunk.
        processor.getStateInformation (chunk);
        chunk.setSize (chunk.getSize() * size_t (rng.range (0, 50)) / 100);
    }
    // mode == 3: leave state untouched this case.
    if (chunk.getSize() > 0)
        processor.setStateInformation (chunk.getData(), int (chunk.getSize()));
}
} // namespace

TEST_CASE ("T-014 seeded state/parameter fuzz produces no crash or NaN", "[frozen][fuzz]")
{
    constexpr uint32_t seed = 0xFC0014u;
    Rng rng (seed);
    const int cases = fuzzCaseCount();
    CAPTURE (seed, cases);

    for (int c = 0; c < cases; ++c)
    {
        CAPTURE (c);
        FaultCounterProcessor processor;
        const double hostRate = double (rng.range (8000, 192000));
        const int numChannels = rng.range (1, 2);
        if (numChannels == 1)
        {
            auto layout = processor.getBusesLayout();
            layout.getChannelSet (true, 0) = juce::AudioChannelSet::mono();
            layout.getChannelSet (false, 0) = juce::AudioChannelSet::mono();
            processor.setBusesLayout (layout);
        }
        processor.prepareToPlay (hostRate, rng.range (16, 8192));

        const int blocks = rng.range (1, 6);
        int sampleOffset = 0;
        for (int b = 0; b < blocks; ++b)
        {
            const int blockSize = rng.range (1, 4096);
            juce::AudioBuffer<float> buffer (numChannels, blockSize);
            for (int channel = 0; channel < numChannels; ++channel)
            {
                auto* data = buffer.getWritePointer (channel);
                for (int i = 0; i < blockSize; ++i, ++sampleOffset)
                {
                    const int mode = rng.range (0, 19);
                    if (mode == 0)
                        data[i] = std::numeric_limits<float>::quiet_NaN();
                    else if (mode == 1)
                        data[i] = std::numeric_limits<float>::infinity() * (rng.unit() < 0.5f ? 1.0f : -1.0f);
                    else
                        data[i] = rng.range (-100, 100) * 0.25f
                                  * float (std::sin (2.0 * 3.14159265358979323846 * double (rng.range (20, 8000))
                                                     * double (sampleOffset) / hostRate));
                }
            }

            if (rng.range (0, 2) == 0)
                randomiseStateChunk (processor, rng);
            randomiseParameters (processor, rng);
            if (rng.range (0, 3) == 0)
                processor.releaseResources();
            else
                processor.prepareToPlay (hostRate, rng.range (16, 8192));

            juce::MidiBuffer midi;
            processor.processBlock (buffer, midi);

            for (int channel = 0; channel < numChannels; ++channel)
            {
                const auto* data = buffer.getReadPointer (channel);
                for (int i = 0; i < blockSize; ++i)
                {
                    CAPTURE (channel, i);
                    REQUIRE (std::isfinite (data[i]));
                }
            }
        }
    }
}
