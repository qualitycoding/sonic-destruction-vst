#include "dsp/Preamp.h"

#include <algorithm>
#include <cmath>

namespace faultcounter
{
void Preamp::setGain (float normalizedGain)
{
    normalizedGain_ = std::clamp (normalizedGain, 0.0f, 1.0f);
}

float Preamp::process (float input) const
{
    const float gainDb = 24.0f * normalizedGain_;
    const float gain = std::pow (10.0f, gainDb / 20.0f);
    const float drive = 8.0f * normalizedGain_ * normalizedGain_;

    if (drive < 1.0e-6f)
        return gain * input;

    return std::tanh (drive * gain * input) / std::tanh (drive);
}
} // namespace faultcounter
