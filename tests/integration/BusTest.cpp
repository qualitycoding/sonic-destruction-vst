// T-012: mono/stereo bus negotiation; stereo channels keep independent state.
#include "PluginProcessor.h"

#include <catch2/catch_test_macros.hpp>

#include <vector>

using namespace faultcounter;

namespace
{
void setNeutralSettings (FaultCounterProcessor& processor)
{
    auto& state = processor.apvts();
    state.getRawParameterValue ("gain")->store (0.0f);
    state.getRawParameterValue ("toneEnabled")->store (0.0f);
    state.getRawParameterValue ("sampleRate")->store (58000.0f);
    state.getRawParameterValue ("bitControl")->store (1.0f);
    state.getRawParameterValue ("table")->store (0.0f);
}

std::vector<float> renderMonoReference (const std::vector<float>& input)
{
    FaultCounterProcessor processor;
    setNeutralSettings (processor);
    // Seed from the current layout and mutate via getChannelSet(): the
    // getMain*ChannelSet() accessors are const-only in JUCE 9 and return by
    // value, so assigning through them would silently discard the change.
    auto layout = processor.getBusesLayout();
    layout.getChannelSet (true, 0) = juce::AudioChannelSet::mono();
    layout.getChannelSet (false, 0) = juce::AudioChannelSet::mono();
    REQUIRE (processor.setBusesLayout (layout));
    processor.prepareToPlay (48000.0, 64);

    juce::AudioBuffer<float> buffer (1, int (input.size()));
    buffer.copyFrom (0, 0, input.data(), int (input.size()));
    juce::MidiBuffer midi;
    processor.processBlock (buffer, midi);
    return { buffer.getReadPointer (0), buffer.getReadPointer (0) + input.size() };
}
} // namespace

TEST_CASE ("T-012 mono and stereo buses negotiate correctly", "[frozen][processor][bus]")
{
    FaultCounterProcessor processor;

    auto stereo = processor.getBusesLayout();
    stereo.getChannelSet (true, 0) = juce::AudioChannelSet::stereo();
    stereo.getChannelSet (false, 0) = juce::AudioChannelSet::stereo();
    REQUIRE (processor.isBusesLayoutSupported (stereo));

    auto mono = processor.getBusesLayout();
    mono.getChannelSet (true, 0) = juce::AudioChannelSet::mono();
    mono.getChannelSet (false, 0) = juce::AudioChannelSet::mono();
    REQUIRE (processor.isBusesLayoutSupported (mono));

    auto mismatched = processor.getBusesLayout();
    mismatched.getChannelSet (true, 0) = juce::AudioChannelSet::stereo();
    mismatched.getChannelSet (false, 0) = juce::AudioChannelSet::mono();
    REQUIRE_FALSE (processor.isBusesLayoutSupported (mismatched));

    auto quad = processor.getBusesLayout();
    quad.getChannelSet (true, 0) = juce::AudioChannelSet::quadraphonic();
    quad.getChannelSet (false, 0) = juce::AudioChannelSet::quadraphonic();
    REQUIRE_FALSE (processor.isBusesLayoutSupported (quad));

    REQUIRE (processor.setBusesLayout (mono));
    processor.prepareToPlay (48000.0, 64);
    juce::AudioBuffer<float> buffer (1, 64);
    buffer.clear();
    juce::MidiBuffer midi;
    processor.processBlock (buffer, midi);
}

TEST_CASE ("T-012 stereo channels do not leak state on an impulse-left test", "[frozen][processor][bus]")
{
    constexpr int length = 64;
    std::vector<float> impulse (size_t (length), 0.0f);
    impulse[0] = 1.0f;
    const std::vector<float> zeros (size_t (length), 0.0f);

    const auto expectedLeft = renderMonoReference (impulse);
    const auto expectedRight = renderMonoReference (zeros);

    FaultCounterProcessor processor;
    setNeutralSettings (processor);
    processor.prepareToPlay (48000.0, length);

    juce::AudioBuffer<float> buffer (2, length);
    buffer.copyFrom (0, 0, impulse.data(), length);
    buffer.copyFrom (1, 0, zeros.data(), length);

    juce::MidiBuffer midi;
    processor.processBlock (buffer, midi);

    for (int i = 0; i < length; ++i)
    {
        CAPTURE (i);
        REQUIRE (buffer.getReadPointer (0)[i] == expectedLeft[size_t (i)]);
        REQUIRE (buffer.getReadPointer (1)[i] == expectedRight[size_t (i)]);
    }
}
