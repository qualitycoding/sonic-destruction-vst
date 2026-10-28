#include "dsp/DcBlocker.h"

#include <algorithm>
#include <cmath>

namespace faultcounter
{
void DcBlocker::prepare (double sampleRate)
{
    sampleRate_ = std::max (1.0, sampleRate);
    coefficient_ = float (1.0 - (2.0 * 3.14159265358979323846 * 5.0 / sampleRate_));
    reset();
}

void DcBlocker::reset()
{
    previousInput_ = 0.0f;
    previousOutput_ = 0.0f;
}

float DcBlocker::process (float input)
{
    const float output = input - previousInput_ + coefficient_ * previousOutput_;
    previousInput_ = input;
    previousOutput_ = output;
    return output;
}

float DcBlocker::state() const noexcept
{
    return previousOutput_;
}
} // namespace faultcounter
