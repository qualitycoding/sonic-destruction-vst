#include "dsp/BitOps.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace faultcounter
{
namespace
{
float sanitize (float value)
{
    return std::isfinite (value) ? value : 0.0f;
}
} // namespace

uint8_t toCode (float value)
{
    value = sanitize (value);
    const float saturated = std::clamp ((value + 1.0f) * 0.5f, 0.0f, 1.0f);
    return uint8_t (std::lround (saturated * 255.0f));
}

float fromCode (uint8_t code)
{
    return 2.0f * (float (code) / 255.0f) - 1.0f;
}

uint8_t applyBits (uint8_t code, int bits)
{
    bits = std::clamp (bits, 1, 8);
    const int levels = (1 << bits) - 1;
    const int q = int (std::lround ((double (code) / 255.0) * double (levels)));
    return uint8_t (std::lround ((double (q) / double (levels)) * 255.0));
}

uint8_t applyMask (uint8_t code, uint8_t mask)
{
    return uint8_t (code & mask);
}
} // namespace faultcounter
