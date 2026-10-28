#include "dsp/Tone.h"

#include <algorithm>
#include <cmath>

namespace faultcounter
{
namespace
{
constexpr double pi = 3.141592653589793238462643383279502884;

void makeBiquad (Tone::Biquad& filter, double sampleRate, double frequency, bool highPass)
{
    const double k = std::tan (pi * frequency / sampleRate);
    const double rootTwo = std::sqrt (2.0);
    const double norm = 1.0 / (1.0 + rootTwo * k + k * k);

    if (highPass)
    {
        filter.b0 = norm;
        filter.b1 = -2.0 * filter.b0;
        filter.b2 = filter.b0;
    }
    else
    {
        filter.b0 = k * k * norm;
        filter.b1 = 2.0 * filter.b0;
        filter.b2 = filter.b0;
    }

    filter.a1 = 2.0 * (k * k - 1.0) * norm;
    filter.a2 = (1.0 - rootTwo * k + k * k) * norm;
}
} // namespace

float Tone::Biquad::process (float input)
{
    const double output = b0 * double (input) + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
    x2 = x1;
    x1 = input;
    y2 = y1;
    y1 = output;
    return float (output);
}

void Tone::Biquad::reset()
{
    x1 = x2 = y1 = y2 = 0.0;
}

void Tone::prepare (double sampleRate)
{
    sampleRate_ = std::max (1.0, sampleRate);
    updateFilters();
}

void Tone::setTone (float normalizedTone, bool enabled)
{
    tone_ = std::clamp (normalizedTone, 0.0f, 1.0f);
    enabled_ = enabled;
}

float Tone::process (float input)
{
    if (! enabled_)
        return input;

    if (tone_ < 0.5f)
    {
        const float mix = 2.0f * tone_;
        return (1.0f - mix) * lowPass_.process (input) + mix * input;
    }

    if (tone_ > 0.5f)
    {
        const float mix = 2.0f * (tone_ - 0.5f);
        return (1.0f - mix) * input + mix * highPass_.process (input);
    }

    return input;
}

void Tone::updateFilters()
{
    makeBiquad (lowPass_, sampleRate_, 900.0, false);
    makeBiquad (highPass_, sampleRate_, 1200.0, true);
    lowPass_.reset();
    highPass_.reset();
}
} // namespace faultcounter
