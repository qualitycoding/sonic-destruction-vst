#pragma once

#include "dsp/DcBlocker.h"
#include "dsp/Preamp.h"
#include "dsp/SampleHold.h"
#include "dsp/Tone.h"

#include <array>
#include <cstdint>
#include <vector>

namespace faultcounter
{
enum class BitMode { Bits, Mask };
enum class BitPosition { Post, Pre };

struct EngineSettings
{
    float gain = 0.2f;
    float tone = 0.5f;
    bool toneEnabled = true;
    float sampleRateHz = 58000.0f;
    float bitControl = 1.0f;
    BitMode bitMode = BitMode::Bits;
    BitPosition bitPosition = BitPosition::Post;
    int table = 0;
    bool bypass = false;
};

class Engine
{
public:
    static constexpr int maxChannels = 2;

    void prepare (double sampleRate, const std::vector<uint8_t>& tableData);
    void reset();
    void setSettings (const EngineSettings& settings);
    void process (float* const* channelData, int numChannels, int numSamples);

private:
    struct ChannelState
    {
        Tone tone;
        Preamp preamp;
        SampleHold sampleHold;
        DcBlocker dcBlocker;
    };

    double sampleRate_ = 48000.0;
    const uint8_t* tables_ = nullptr;
    EngineSettings settings_;
    std::array<ChannelState, maxChannels> channels_;
};
} // namespace faultcounter
