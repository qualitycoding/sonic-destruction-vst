// T-013: operational robustness — SC-04 rates, block schedule, repeated
// prepare/reset, interrupted automation, and hostile inputs produce no
// crash, NaN, or Inf.
#include "PluginProcessor.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <vector>

using namespace faultcounter;

namespace
{
constexpr double sc04Rates[] = { 44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0 };
constexpr int blockSchedule[] = { 1, 2, 7, 16, 64, 511, 4096, 8192 };

void fillHostile (juce::AudioBuffer<float>& buffer, int sampleOffset, float amplitude)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* data = buffer.getWritePointer (channel);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const int n = sampleOffset + i;
            // Multi-tone content keeps the table lookup exercised across
            // codes while the amplitude sweeps the SC-06 input range.
            data[i] = amplitude
                      * float (0.6 * std::sin (2.0 * 3.14159265358979323846 * 441.0 * double (n) / 48000.0)
                               + 0.4 * std::sin (2.0 * 3.14159265358979323846 * 7331.0 * double (n) / 48000.0));
        }
    }
}

void requireFinite (const juce::AudioBuffer<float>& buffer)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        const auto* data = buffer.getReadPointer (channel);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            CAPTURE (channel, i);
            REQUIRE (std::isfinite (data[i]));
        }
    }
}

void thrashParameters (FaultCounterProcessor& processor, int step)
{
    auto& state = processor.apvts();
    const float u = float ((step * 37 % 101)) / 100.0f;
    state.getRawParameterValue ("gain")->store (u);
    state.getRawParameterValue ("tone")->store (1.0f - u);
    state.getRawParameterValue ("toneEnabled")->store (step % 2 == 0 ? 1.0f : 0.0f);
    state.getRawParameterValue ("sampleRate")->store (280.0f + u * 57720.0f);
    state.getRawParameterValue ("fine")->store (step % 3 == 0 ? 1.0f : 0.0f);
    state.getRawParameterValue ("bitControl")->store (u);
    state.getRawParameterValue ("bitMode")->store (step % 2 == 1 ? 1.0f : 0.0f);
    state.getRawParameterValue ("bitPosition")->store (step % 2 == 0 ? 1.0f : 0.0f);
    state.getRawParameterValue ("table")->store (float (step * 13 % 252));
    state.getRawParameterValue ("bypass")->store (step % 5 == 0 ? 1.0f : 0.0f);
}

// Begins a gesture on every parameter and deliberately never ends half of
// them: hosts can be interrupted mid-automation and the plug-in must stay
// well-defined.
void interruptAutomation (FaultCounterProcessor& processor, int step)
{
    auto& state = processor.apvts();
    for (const char* id : { "gain", "tone", "sampleRate", "bitControl", "table" })
    {
        if (auto* parameter = state.getParameter (id))
        {
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost (float ((step * 53 % 97)) / 97.0f);
            if ((step + id[0]) % 2 == 0)
                parameter->endChangeGesture();
        }
    }
}
} // namespace

TEST_CASE ("T-013 no crash or NaN across SC-04 rates and the block schedule", "[frozen][robustness]")
{
    for (const double hostRate : sc04Rates)
    {
        for (const int numChannels : { 1, 2 })
        {
            CAPTURE (hostRate, numChannels);
            FaultCounterProcessor processor;
            if (numChannels == 1)
            {
                auto layout = processor.getBusesLayout();
                layout.getChannelSet (true, 0) = juce::AudioChannelSet::mono();
                layout.getChannelSet (false, 0) = juce::AudioChannelSet::mono();
                REQUIRE (processor.setBusesLayout (layout));
            }

            for (int round = 0; round < 2; ++round)
            {
                processor.prepareToPlay (hostRate, 8192);
                int sampleOffset = 0;
                float amplitude = 1.0f;
                for (const int blockSize : blockSchedule)
                {
                    CAPTURE (round, blockSize);
                    // Amplitude sweeps the SC-06 finite-input range, which
                    // includes hot (+/-100) transients between blocks.
                    amplitude = amplitude >= 100.0f ? 1.0f : amplitude * 1.9f;
                    juce::AudioBuffer<float> buffer (numChannels, blockSize);
                    fillHostile (buffer, sampleOffset, amplitude);
                    sampleOffset += blockSize;

                    thrashParameters (processor, sampleOffset + round);
                    juce::MidiBuffer midi;
                    processor.processBlock (buffer, midi);
                    requireFinite (buffer);
                }
                processor.releaseResources();
            }
        }
    }
}

TEST_CASE ("T-013 hostile NaN and Inf inputs never escape", "[frozen][robustness]")
{
    FaultCounterProcessor processor;
    processor.prepareToPlay (48000.0, 512);

    for (const int step : { 0, 1, 2 })
    {
        CAPTURE (step);
        juce::AudioBuffer<float> buffer (2, 511);
        fillHostile (buffer, step * 511, 2.0f);
        buffer.setSample (0, 10, std::numeric_limits<float>::quiet_NaN());
        buffer.setSample (1, 20, std::numeric_limits<float>::infinity());
        buffer.setSample (0, 30, -std::numeric_limits<float>::infinity());

        juce::MidiBuffer midi;
        processor.processBlock (buffer, midi);
        requireFinite (buffer);
    }
}

TEST_CASE ("T-013 interrupted automation stays well-defined", "[frozen][robustness]")
{
    FaultCounterProcessor processor;
    processor.prepareToPlay (96000.0, 1024);

    int sampleOffset = 0;
    for (int step = 0; step < 16; ++step)
    {
        CAPTURE (step);
        juce::AudioBuffer<float> buffer (2, 64);
        fillHostile (buffer, sampleOffset, 1.0f);
        sampleOffset += 64;

        interruptAutomation (processor, step);
        juce::MidiBuffer midi;
        processor.processBlock (buffer, midi);
        requireFinite (buffer);
    }
}
