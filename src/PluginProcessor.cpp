// S-002 real implementation — JUCE 9 VST3 plug-in processor (D-003/D-007/D-009).
#include "PluginProcessor.h"

#include "Parameters.h"
#include "dsp/TablesData.h"
#include "ui/PluginEditor.h"

#include <cmath>
#include <cstring>

namespace faultcounter
{
namespace
{
constexpr unsigned kStateMagic = 0x544e4346u; // "FCNT" little-endian
constexpr unsigned kStateVersion = 1;
constexpr size_t kStateHeaderBytes = 12;
constexpr size_t kMaxStateChunkBytes = 1024 * 1024;

void appendUint32 (juce::MemoryBlock& block, unsigned value)
{
    const unsigned char bytes[] = { static_cast<unsigned char> (value & 0xffu), static_cast<unsigned char> ((value >> 8) & 0xffu),
                                    static_cast<unsigned char> ((value >> 16) & 0xffu), static_cast<unsigned char> ((value >> 24) & 0xffu) };
    block.append (bytes, 4);
}

unsigned readUint32 (const unsigned char* bytes)
{
    return unsigned (bytes[0]) | (unsigned (bytes[1]) << 8) | (unsigned (bytes[2]) << 16)
           | (unsigned (bytes[3]) << 24);
}

juce::String plainNumberText (float value)
{
    return juce::String (value, 2);
}

float parseClamped01 (const juce::String& text, float fallback)
{
    const auto parsed = text.getFloatValue();
    return std::isfinite (parsed) ? juce::jlimit (0.0f, 1.0f, parsed) : fallback;
}

juce::NormalisableRange<float> sampleRateRange()
{
    return juce::NormalisableRange<float> (
        280.0f,
        58000.0f,
        [] (float start, float, float value) {
            return start * std::pow (58000.0f / 280.0f, juce::jlimit (0.0f, 1.0f, value));
        },
        [] (float start, float end, float value) {
            const auto clamped = juce::jlimit (start, end, std::isfinite (value) ? value : end);
            return juce::jlimit (0.0f, 1.0f, std::log (clamped / start) / std::log (end / start));
        });
}

float sampleRateTextToValue (const juce::String& text)
{
    auto trimmed = text.trim();
    float factor = 1.0f;
    if (trimmed.endsWithIgnoreCase ("khz"))
    {
        factor = 1000.0f;
        trimmed = trimmed.dropLastCharacters (3).trim();
    }
    else if (trimmed.endsWithIgnoreCase ("hz"))
    {
        trimmed = trimmed.dropLastCharacters (2).trim();
    }
    const auto parsed = trimmed.getFloatValue() * factor;
    return std::isfinite (parsed) ? juce::jlimit (280.0f, 58000.0f, parsed) : 58000.0f;
}

juce::String tableValueToText (int value, int)
{
    return tableDisplay (value);
}

// APVTS serialises denormalised values in state XML (juce_AudioProcessorValueTreeState.cpp
// stores unnormalisedValue), so validation must use each parameter's actual range.
bool knownParamRange (const juce::String& id, float& lo, float& hi)
{
    if (id == "sampleRate")
    {
        lo = 280.0f;
        hi = 58000.0f;
        return true;
    }
    if (id == "table")
    {
        lo = 0.0f;
        hi = 251.0f;
        return true;
    }
    for (const char* pid : { "gain", "tone", "toneEnabled", "fine", "bitControl", "bitMode", "bitPosition", "bypass" })
    {
        if (id == pid)
        {
            lo = 0.0f;
            hi = 1.0f;
            return true;
        }
    }
    return false;
}

float tableTextToValue (const juce::String& text)
{
    const auto trimmed = text.trim();
    if (trimmed.isEmpty())
        return 0.0f;
    const auto hex = trimmed.startsWithIgnoreCase ("0x") ? trimmed.substring (2) : trimmed;
    if (! hex.isEmpty() && hex.containsOnly ("0123456789abcdefABCDEF"))
    {
        const auto asHex = hex.getHexValue32();
        if (asHex >= 0 && asHex <= 251)
            return float (asHex);
    }
    return float (juce::jlimit (0, 251, trimmed.getIntValue()));
}
} // namespace

FaultCounterProcessor::FaultCounterProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_ (*this, nullptr, "faultcounter", createParameterLayout (currentBitMode_))
{
    apvts_.addParameterListener ("bitMode", this);
    pullSettings();
}

FaultCounterProcessor::~FaultCounterProcessor()
{
    apvts_.removeParameterListener ("bitMode", this);
}

juce::AudioProcessorValueTreeState::ParameterLayout FaultCounterProcessor::createParameterLayout (std::atomic<int>& currentBitMode)
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto makeFloat = [&] (const char* id, const char* name, float min, float max, float def,
                          std::function<juce::String (float, int)> toText,
                          std::function<float (const juce::String&)> fromText) {
        juce::AudioParameterFloatAttributes attributes;
        attributes = attributes.withLabel (name)
                                 .withStringFromValueFunction (std::move (toText))
                                 .withValueFromStringFunction (std::move (fromText));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { id, 1 },
                                                                       name,
                                                                       juce::NormalisableRange<float> (min, max),
                                                                       def,
                                                                       attributes));
    };

    makeFloat ("gain", "Gain", 0.0f, 1.0f, 0.2f,
               [] (float v, int) { return plainNumberText (v); },
               [] (const juce::String& s) { return parseClamped01 (s, 0.2f); });
    makeFloat ("tone", "Tone", 0.0f, 1.0f, 0.5f,
               [] (float v, int) { return plainNumberText (v); },
               [] (const juce::String& s) { return parseClamped01 (s, 0.5f); });

    params.push_back (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "toneEnabled", 1 },
                                                                  "Tone Enabled", true));
    {
        juce::AudioParameterFloatAttributes attributes;
        attributes = attributes.withLabel ("Sample Rate")
                                 .withStringFromValueFunction ([] (float hz, int) { return sampleRateDisplay (hz); })
                                 .withValueFromStringFunction (sampleRateTextToValue);
        params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "sampleRate", 1 },
                                                                       "Sample Rate", sampleRateRange(), 58000.0f,
                                                                       attributes));
    }
    params.push_back (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "fine", 1 }, "Fine", false));
    {
        juce::AudioParameterFloatAttributes attributes;
        attributes = attributes.withLabel ("Bit Control")
                                 .withStringFromValueFunction ([&currentBitMode] (float v, int) {
                                     return bitDisplay (v, currentBitMode.load() == 1);
                                 })
                                 .withValueFromStringFunction ([&currentBitMode] (const juce::String& s) {
                                     if (currentBitMode.load() == 1)
                                         return juce::jlimit (0.0f, 1.0f, s.trim().getIntValue() / 255.0f);
                                     return parseClamped01 (s, 1.0f);
                                 });
        params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "bitControl", 1 },
                                                                       "Bit Control",
                                                                       juce::NormalisableRange<float> (0.0f, 1.0f),
                                                                       1.0f, attributes));
    }
    params.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "bitMode", 1 },
                                                                      "Bit Mode", juce::StringArray { "Bits", "Mask" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "bitPosition", 1 },
                                                                      "Bit Position", juce::StringArray { "Post", "Pre" }, 0));
    {
        juce::AudioParameterIntAttributes attributes;
        attributes = attributes.withLabel ("Table")
                                 .withStringFromValueFunction (tableValueToText)
                                 .withValueFromStringFunction (tableTextToValue);
        params.push_back (std::make_unique<juce::AudioParameterInt> (juce::ParameterID { "table", 1 },
                                                                     "Table", 0, 251, 0, attributes));
    }
    params.push_back (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "bypass", 1 },
                                                                  "Bypass", false));

    return { params.begin(), params.end() };
}

void FaultCounterProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    if (parameterID == "bitMode")
        currentBitMode_ = int (std::lround (newValue));
}

void FaultCounterProcessor::pullSettings()
{
    auto& state = apvts_;
    settings_.gain = state.getRawParameterValue ("gain")->load();
    settings_.tone = state.getRawParameterValue ("tone")->load();
    settings_.toneEnabled = state.getRawParameterValue ("toneEnabled")->load() >= 0.5f;
    settings_.sampleRateHz = juce::jlimit (280.0f, 58000.0f, state.getRawParameterValue ("sampleRate")->load());
    settings_.bitControl = state.getRawParameterValue ("bitControl")->load();
    settings_.bitMode = state.getRawParameterValue ("bitMode")->load() >= 0.5f ? BitMode::Mask : BitMode::Bits;
    settings_.bitPosition = state.getRawParameterValue ("bitPosition")->load() >= 0.5f ? BitPosition::Pre : BitPosition::Post;
    settings_.table = int (std::lround (state.getRawParameterValue ("table")->load()));
    settings_.bypass = state.getRawParameterValue ("bypass")->load() >= 0.5f;

    if (! std::isfinite (settings_.gain))
        settings_.gain = 0.2f;
    if (! std::isfinite (settings_.tone))
        settings_.tone = 0.5f;
    if (! std::isfinite (settings_.bitControl))
        settings_.bitControl = 1.0f;
}

void FaultCounterProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    hostSampleRate_ = sampleRate;
    hostBlockSize_ = samplesPerBlock;

    pullSettings();

    gainSmoothed_.reset (sampleRate, 0.005);
    toneSmoothed_.reset (sampleRate, 0.005);
    sampleRateSmoothed_.reset (sampleRate, 0.001);
    gainSmoothed_.setCurrentAndTargetValue (settings_.gain);
    toneSmoothed_.setCurrentAndTargetValue (settings_.tone);
    sampleRateSmoothed_.setCurrentAndTargetValue (settings_.sampleRateHz);

    const auto* tablesBegin = reinterpret_cast<const uint8_t*> (kEmbeddedTables);
    engine_.prepare (sampleRate, std::vector<uint8_t> (tablesBegin, tablesBegin + kEmbeddedTablesSize));
    engine_.setSettings (settings_);
}

void FaultCounterProcessor::releaseResources() {}

bool FaultCounterProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    const auto set = layouts.getMainInputChannelSet();
    return set == juce::AudioChannelSet::mono() || set == juce::AudioChannelSet::stereo();
}

void FaultCounterProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    for (int channel = totalNumInputChannels; channel < totalNumOutputChannels; ++channel)
        buffer.clear (channel, 0, buffer.getNumSamples());

    pullSettings();

    if (settings_.bypass)
    {
        // In-place buffer: input is already the output (bit-identical for
        // finite samples). Hostile NaN/Inf samples are zeroed so nothing
        // non-finite escapes (SC-06); identity holds for all finite input.
        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
        {
            auto* data = buffer.getWritePointer (channel);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                if (! std::isfinite (data[i]))
                    data[i] = 0.0f;
        }
        return;
    }

    gainSmoothed_.setTargetValue (settings_.gain);
    toneSmoothed_.setTargetValue (settings_.tone);
    sampleRateSmoothed_.setTargetValue (settings_.sampleRateHz);

    // Block-granularity smoothing keeps the engine settings consistent across channels.
    EngineSettings smoothed = settings_;
    smoothed.gain = gainSmoothed_.getNextValue();
    smoothed.tone = toneSmoothed_.getNextValue();
    smoothed.sampleRateHz = sampleRateSmoothed_.getNextValue();
    engine_.setSettings (smoothed);

    const int channelsToProcess = std::min (totalNumInputChannels, totalNumOutputChannels);
    engine_.process (buffer.getArrayOfWritePointers(), channelsToProcess, buffer.getNumSamples());
}

juce::AudioProcessorEditor* FaultCounterProcessor::createEditor()
{
    return new FaultCounterEditor (*this);
}

void FaultCounterProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Flush the live parameter values into the copied tree: callers may set
    // values through getRawParameterValue() (no host notification), and the
    // state chunk must reflect what processBlock() actually uses.
    auto state = apvts_.copyState();
    for (const auto& descriptor : parameterDescriptors())
    {
        const auto id = juce::String (descriptor.id.data(), descriptor.id.size());
        auto* raw = apvts_.getRawParameterValue (id);
        if (raw == nullptr)
            continue;
        if (auto child = state.getChildWithProperty ("id", id); child.isValid())
            child.setProperty ("value", raw->load(), nullptr);
    }

    if (auto xml = state.createXml())
    {
        const auto payload = xml->toString();
        destData.reset();
        appendUint32 (destData, kStateMagic);
        appendUint32 (destData, kStateVersion);
        appendUint32 (destData, unsigned (payload.getNumBytesAsUTF8()));
        destData.append (payload.toRawUTF8(), payload.getNumBytesAsUTF8());
    }
}

void FaultCounterProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (data == nullptr || sizeInBytes < int (kStateHeaderBytes) || size_t (sizeInBytes) > kMaxStateChunkBytes)
        return;

    const auto* bytes = static_cast<const unsigned char*> (data);
    if (readUint32 (bytes) != kStateMagic || readUint32 (bytes + 4) != kStateVersion)
        return;

    const auto payloadLength = readUint32 (bytes + 8);
    if (payloadLength != size_t (sizeInBytes) - kStateHeaderBytes)
        return;

    const auto* payload = reinterpret_cast<const char*> (bytes + kStateHeaderBytes);
    std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (juce::String::fromUTF8 (payload, int (payloadLength))));
    if (xml == nullptr || ! xml->hasTagName ("faultcounter"))
        return;

    for (auto* param : xml->getChildIterator())
    {
        if (! param->hasAttribute ("id") || ! param->hasAttribute ("value"))
            continue;
        const auto id = param->getStringAttribute ("id");

        float lo = 0.0f, hi = 1.0f;
        if (! knownParamRange (id, lo, hi))
            continue; // forward tolerance: unknown parameters are ignored

        const auto value = param->getDoubleAttribute ("value", std::numeric_limits<double>::quiet_NaN());
        if (! std::isfinite (value) || float (value) < lo || float (value) > hi)
            return;
    }

    const auto restored = juce::ValueTree::fromXml (*xml);
    if (! restored.isValid())
        return;

    apvts_.replaceState (restored);
    pullSettings();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FaultCounterProcessor();
}
} // namespace faultcounter

// JUCE's plug-in client looks up createPluginFilter() in the global namespace.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return faultcounter::createPluginFilter();
}
