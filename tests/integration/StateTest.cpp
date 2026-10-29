// T-011: state round trips preserve every parameter; malformed chunks fail safely.
#include "PluginProcessor.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cstring>
#include <vector>

using namespace faultcounter;
using Catch::Matchers::WithinAbs;

namespace
{
void setNonDefaultState (FaultCounterProcessor& processor)
{
    auto& state = processor.apvts();
    state.getRawParameterValue ("gain")->store (0.73f);
    state.getRawParameterValue ("tone")->store (0.31f);
    state.getRawParameterValue ("toneEnabled")->store (0.0f);
    state.getRawParameterValue ("sampleRate")->store (9000.0f);
    state.getRawParameterValue ("fine")->store (1.0f);
    state.getRawParameterValue ("bitControl")->store (0.25f);
    state.getRawParameterValue ("bitMode")->store (1.0f);
    state.getRawParameterValue ("bitPosition")->store (1.0f);
    state.getRawParameterValue ("table")->store (42.0f);
    state.getRawParameterValue ("bypass")->store (1.0f);
}

void requireNonDefaultState (FaultCounterProcessor& processor)
{
    auto& state = processor.apvts();
    REQUIRE_THAT (double (state.getRawParameterValue ("gain")->load()), WithinAbs (0.73, 1e-6));
    REQUIRE_THAT (double (state.getRawParameterValue ("tone")->load()), WithinAbs (0.31, 1e-6));
    REQUIRE (state.getRawParameterValue ("toneEnabled")->load() == 0.0f);
    REQUIRE_THAT (double (state.getRawParameterValue ("sampleRate")->load()), WithinAbs (9000.0, 1e-1));
    REQUIRE (state.getRawParameterValue ("fine")->load() == 1.0f);
    REQUIRE_THAT (double (state.getRawParameterValue ("bitControl")->load()), WithinAbs (0.25, 1e-6));
    REQUIRE (state.getRawParameterValue ("bitMode")->load() == 1.0f);
    REQUIRE (state.getRawParameterValue ("bitPosition")->load() == 1.0f);
    REQUIRE (state.getRawParameterValue ("table")->load() == 42.0f);
    REQUIRE (state.getRawParameterValue ("bypass")->load() == 1.0f);
}

void requireDefaultState (FaultCounterProcessor& processor)
{
    auto& state = processor.apvts();
    REQUIRE_THAT (double (state.getRawParameterValue ("gain")->load()), WithinAbs (0.2, 1e-6));
    REQUIRE_THAT (double (state.getRawParameterValue ("tone")->load()), WithinAbs (0.5, 1e-6));
    REQUIRE (state.getRawParameterValue ("bypass")->load() == 0.0f);
}

juce::MemoryBlock makeChunk (const juce::MemoryBlock& valid)
{
    return valid;
}
} // namespace

TEST_CASE ("T-011 state round trip preserves every parameter", "[frozen][processor][state]")
{
    FaultCounterProcessor source;
    setNonDefaultState (source);

    juce::MemoryBlock chunk;
    source.getStateInformation (chunk);
    REQUIRE (chunk.getSize() > 12);

    FaultCounterProcessor restored;
    restored.setStateInformation (chunk.getData(), int (chunk.getSize()));
    requireNonDefaultState (restored);

    // A second round trip is stable.
    juce::MemoryBlock chunk2;
    restored.getStateInformation (chunk2);
    FaultCounterProcessor restored2;
    restored2.setStateInformation (chunk2.getData(), int (chunk2.getSize()));
    requireNonDefaultState (restored2);
}

TEST_CASE ("T-011 malformed chunks fail safely", "[frozen][processor][state]")
{
    FaultCounterProcessor source;
    setNonDefaultState (source);
    juce::MemoryBlock valid;
    source.getStateInformation (valid);

    const auto expectRejected = [&valid] (std::function<void (juce::MemoryBlock&)> mutate) {
        FaultCounterProcessor target;
        auto chunk = makeChunk (valid);
        mutate (chunk);
        target.setStateInformation (chunk.getData(), int (chunk.getSize()));
        requireDefaultState (target);
    };

    // Wrong magic.
    expectRejected ([] (juce::MemoryBlock& chunk) {
        auto* bytes = static_cast<unsigned char*> (chunk.getData());
        bytes[0] = 'X';
    });

    // Unsupported schema version.
    expectRejected ([] (juce::MemoryBlock& chunk) {
        auto* bytes = static_cast<unsigned char*> (chunk.getData());
        bytes[4] = 2;
    });

    // Truncated header.
    {
        FaultCounterProcessor target;
        auto chunk = makeChunk (valid);
        target.setStateInformation (chunk.getData(), 6);
        requireDefaultState (target);
    }

    // Payload length mismatch (corrupt length field).
    expectRejected ([] (juce::MemoryBlock& chunk) {
        auto* bytes = static_cast<unsigned char*> (chunk.getData());
        bytes[8] = 0xff;
    });

    // Garbage payload with valid header.
    expectRejected ([] (juce::MemoryBlock& chunk) {
        auto* bytes = static_cast<unsigned char*> (chunk.getData());
        for (size_t i = 12; i < chunk.getSize(); ++i)
            bytes[i] = char (i & 0xff);
    });

    // Out-of-range parameter value in otherwise valid XML. Locate the gain
    // PARAM and corrupt the first digit of its value attribute (JUCE
    // serialises floats at full precision, so a fixed literal needle is not
    // guaranteed to match — see TEST_CHALLENGE defect 8).
    expectRejected ([] (juce::MemoryBlock& chunk) {
        auto* bytes = static_cast<unsigned char*> (chunk.getData());
        const std::string payload (reinterpret_cast<const char*> (bytes) + 12, chunk.getSize() - 12);
        const auto idPos = payload.find ("id=\"gain\"");
        REQUIRE (idPos != std::string::npos);
        const auto valuePos = payload.find ("value=\"", idPos);
        REQUIRE (valuePos != std::string::npos);
        bytes[12 + valuePos + 7] = '9'; // 0.73... -> 9.73..., above gain's [0,1] range
    });

    // Oversized chunk is rejected without reading the payload.
    {
        FaultCounterProcessor target;
        std::vector<char> oversized (1024 * 1024 + 16, 0);
        std::memcpy (oversized.data(), valid.getData(), std::min (valid.getSize(), size_t (16)));
        target.setStateInformation (oversized.data(), int (oversized.size()));
        requireDefaultState (target);
    }

    // Null / empty input.
    {
        FaultCounterProcessor target;
        target.setStateInformation (nullptr, 0);
        target.setStateInformation (valid.getData(), 0);
        requireDefaultState (target);
    }
}

TEST_CASE ("T-011 unrecognized state elements are ignored for forward tolerance", "[frozen][processor][state]")
{
    FaultCounterProcessor source;
    setNonDefaultState (source);
    juce::MemoryBlock chunk;
    source.getStateInformation (chunk);

    // Inject an unknown parameter element into the XML payload.
    FaultCounterProcessor target;
    auto* bytes = static_cast<unsigned char*> (chunk.getData());
    const std::string needle = "<PARAM";
    const auto payload = std::string (reinterpret_cast<const char*> (bytes) + 12, chunk.getSize() - 12);
    const auto pos = payload.find (needle);
    REQUIRE (pos != std::string::npos);

    const std::string injected = "<PARAM id=\"futureParam\" value=\"0.5\"/>" + payload.substr (pos);
    const std::string prefix = payload.substr (0, pos);

    juce::MemoryBlock modified;
    auto append32 = [&modified] (unsigned value) {
        const unsigned char b[] = { static_cast<unsigned char> (value & 0xff), static_cast<unsigned char> ((value >> 8) & 0xff),
                                    static_cast<unsigned char> ((value >> 16) & 0xff), static_cast<unsigned char> ((value >> 24) & 0xff) };
        modified.append (b, 4);
    };
    append32 (0x544e4346u);
    append32 (1u);
    append32 (unsigned (prefix.size() + injected.size()));
    modified.append (prefix.data(), prefix.size());
    modified.append (injected.data(), injected.size());

    target.setStateInformation (modified.getData(), int (modified.getSize()));
    requireNonDefaultState (target);
}
