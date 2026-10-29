// T-010: bypass is bit-identical for mono/stereo and all block sizes.
#include "PluginProcessor.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <vector>

using namespace faultcounter;

namespace
{
void fillDeterministic (juce::AudioBuffer<float>& buffer, int sampleOffset)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* data = buffer.getWritePointer (channel);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const int n = sampleOffset + i;
            data[i] = float (0.6 * std::sin (2.0 * 3.14159265358979323846 * 441.0 * double (n) / 48000.0)
                             + 0.2 * std::sin (2.0 * 3.14159265358979323846 * 1337.0 * double (n) / 48000.0));
        }
    }
}

void runBypassCase (int numChannels)
{
    FaultCounterProcessor processor;
    if (numChannels == 1)
    {
        auto layout = processor.getBusesLayout();
        layout.getChannelSet (true, 0) = juce::AudioChannelSet::mono();
        layout.getChannelSet (false, 0) = juce::AudioChannelSet::mono();
        REQUIRE (processor.setBusesLayout (layout));
    }

    processor.apvts().getRawParameterValue ("bypass")->store (1.0f);
    processor.prepareToPlay (48000.0, 8192);

    int sampleOffset = 0;
    for (const int blockSize : { 1, 2, 7, 16, 64, 511, 4096, 8192 })
    {
        CAPTURE (numChannels, blockSize);
        juce::AudioBuffer<float> buffer (numChannels, blockSize);
        fillDeterministic (buffer, sampleOffset);
        sampleOffset += blockSize;

        std::vector<std::vector<float>> expected ((size_t (numChannels)));
        for (int channel = 0; channel < numChannels; ++channel)
            expected[size_t (channel)] = std::vector<float> (buffer.getReadPointer (channel),
                                                             buffer.getReadPointer (channel) + blockSize);

        juce::MidiBuffer midi;
        processor.processBlock (buffer, midi);

        for (int channel = 0; channel < numChannels; ++channel)
        {
            const auto* data = buffer.getReadPointer (channel);
            for (int i = 0; i < blockSize; ++i)
            {
                CAPTURE (channel, i);
                REQUIRE (data[i] == expected[size_t (channel)][size_t (i)]);
            }
        }
    }
}
} // namespace

TEST_CASE ("T-010 bypass is bit-identical", "[frozen][processor]")
{
    runBypassCase (1);
    runBypassCase (2);
}

TEST_CASE ("T-010 non-bypassed processing actually processes", "[frozen][processor]")
{
    FaultCounterProcessor processor;
    processor.prepareToPlay (48000.0, 512);

    juce::AudioBuffer<float> buffer (2, 512);
    fillDeterministic (buffer, 0);

    std::vector<std::vector<float>> input (2);
    for (int channel = 0; channel < 2; ++channel)
        input[size_t (channel)] = std::vector<float> (buffer.getReadPointer (channel), buffer.getReadPointer (channel) + 512);

    juce::MidiBuffer midi;
    processor.processBlock (buffer, midi);

    bool anyDifference = false;
    for (int channel = 0; channel < 2; ++channel)
    {
        const auto* data = buffer.getReadPointer (channel);
        for (int i = 0; i < 512; ++i)
            anyDifference |= (data[i] != input[size_t (channel)][size_t (i)]);
    }
    REQUIRE (anyDifference);
}
