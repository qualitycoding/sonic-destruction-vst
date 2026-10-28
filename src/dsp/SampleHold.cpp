#include "dsp/SampleHold.h"

#include <algorithm>
#include <cmath>

namespace faultcounter
{
void SampleHold::prepare (double sampleRate)
{
    hostRate_ = std::max (1.0, sampleRate);
    reset();
}

void SampleHold::reset()
{
    phase_ = 1.0;
    held_ = 0.0f;
}

float SampleHold::process (float input, double requestedRate)
{
    const double effectiveRate = std::clamp (requestedRate, 1.0, hostRate_);
    phase_ += effectiveRate / hostRate_;

    if (phase_ >= 1.0)
    {
        held_ = std::isfinite (input) ? input : 0.0f;
        phase_ -= std::floor (phase_);
    }

    return held_;
}

double SampleHold::phase() const noexcept
{
    return phase_;
}

float SampleHold::heldValue() const noexcept
{
    return held_;
}
} // namespace faultcounter
