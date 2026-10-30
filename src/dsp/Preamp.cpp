#include "dsp/Preamp.h"

#include <algorithm>
#include <cmath>

namespace faultcounter
{
Preamp::Preamp()
{
    setGain (normalizedGain_);
}

void Preamp::setGain (float normalizedGain)
{
    normalizedGain_ = std::clamp (normalizedGain, 0.0f, 1.0f);

    const float gainDb = 24.0f * normalizedGain_;
    linearGain_ = std::pow (10.0f, gainDb / 20.0f);
    drive_ = 8.0f * normalizedGain_ * normalizedGain_;
    cleanGain_ = drive_ < 1.0e-6f;
    tanhDrive_ = std::tanh (drive_);
}

float Preamp::process (float input) const
{
    if (cleanGain_)
        return linearGain_ * input;

    return std::tanh (drive_ * linearGain_ * input) / tanhDrive_;
}
} // namespace faultcounter
