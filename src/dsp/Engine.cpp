#include "dsp/Engine.h"

#include "Parameters.h"
#include "dsp/BitOps.h"
#include "dsp/Tables.h"

#include <algorithm>
#include <cmath>

namespace faultcounter
{
namespace
{
float sanitize (float value)
{
    return std::isfinite (value) ? value : 0.0f;
}

uint8_t reduceCode (uint8_t code, const EngineSettings& settings)
{
    return settings.bitMode == BitMode::Bits ? applyBits (code, bitControlToBits (settings.bitControl)) : applyMask (code, uint8_t (bitControlToMask (settings.bitControl)));
}
} // namespace

void Engine::prepare (double sampleRate, const std::vector<uint8_t>& tableData)
{
    sampleRate_ = std::max (1.0, sampleRate);
    tables_ = tableData;
    if (tables_.size() < tableRowLength)
        tables_.resize (tableRowLength, 0);

    for (auto& channel : channels_)
    {
        channel.tone.prepare (sampleRate_);
        channel.preamp.setGain (settings_.gain);
        channel.sampleHold.prepare (sampleRate_);
        channel.dcBlocker.prepare (sampleRate_);
    }
}

void Engine::reset()
{
    for (auto& channel : channels_)
    {
        channel.tone.setTone (settings_.tone, settings_.toneEnabled);
        channel.sampleHold.reset();
        channel.dcBlocker.reset();
    }
}

void Engine::setSettings (const EngineSettings& settings)
{
    settings_ = settings;
    settings_.gain = std::clamp (settings_.gain, 0.0f, 1.0f);
    settings_.tone = std::clamp (settings_.tone, 0.0f, 1.0f);
    settings_.sampleRateHz = std::clamp (settings_.sampleRateHz, 1.0f, 58000.0f);
    settings_.bitControl = std::clamp (settings_.bitControl, 0.0f, 1.0f);
    settings_.table = std::clamp (settings_.table, 0, 251);

    for (auto& channel : channels_)
    {
        channel.preamp.setGain (settings_.gain);
        channel.tone.setTone (settings_.tone, settings_.toneEnabled);
    }
}

void Engine::process (float* const* channelData, int numChannels, int numSamples)
{
    if (channelData == nullptr || numChannels <= 0 || numSamples <= 0)
        return;

    const int channelsToProcess = std::min (numChannels, maxChannels);
    for (int channel = 0; channel < channelsToProcess; ++channel)
    {
        float* data = channelData[channel];
        if (data == nullptr)
            continue;

        auto& state = channels_[size_t (channel)];

        if (settings_.bypass)
        {
            for (int i = 0; i < numSamples; ++i)
                if (! std::isfinite (data[i]))
                    data[i] = 0.0f;
            continue;
        }

        // setSettings() already pushes gain/tone into every channel; no
        // per-block re-application needed here.

        for (int i = 0; i < numSamples; ++i)
        {
            float sample = sanitize (data[i]);
            sample = state.preamp.process (sample);
            sample = state.tone.process (sample);
            sample = state.sampleHold.process (sample, double (settings_.sampleRateHz));

            uint8_t code = toCode (sample);
            if (settings_.bitPosition == BitPosition::Pre)
                code = reduceCode (code, settings_);

            const int table = std::clamp (settings_.table, 0, int (tables_.size() / tableRowLength) - 1);
            code = tables_[size_t (table) * tableRowLength + code];

            if (settings_.bitPosition == BitPosition::Post)
                code = reduceCode (code, settings_);

            sample = fromCode (code);
            sample = state.dcBlocker.process (sample);
            data[i] = std::isfinite (sample) ? sample : 0.0f;
        }
    }
}
} // namespace faultcounter
