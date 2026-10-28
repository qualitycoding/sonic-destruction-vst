#include "Parameters.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace faultcounter
{
namespace
{
constexpr std::array<ParameterDescriptor, 10> descriptors {{
    { "gain",           ParameterKind::Float, 0.0f, 1.0f,     0.2f,  0, 0 },
    { "tone",           ParameterKind::Float, 0.0f, 1.0f,     0.5f,  0, 0 },
    { "toneEnabled",    ParameterKind::Bool,  0.0f, 1.0f,     1.0f,  0, 0 },
    { "sampleRate",     ParameterKind::Float, 280.0f, 58000.0f, 58000.0f, 0, 0 },
    { "fine",           ParameterKind::Bool,  0.0f, 1.0f,     0.0f,  0, 0 },
    { "bitControl",     ParameterKind::Float, 0.0f, 1.0f,     1.0f,  0, 0 },
    { "bitMode",        ParameterKind::Choice, 0.0f, 1.0f,    0.0f,  0, 2 },
    { "bitPosition",    ParameterKind::Choice, 0.0f, 1.0f,    0.0f,  0, 2 },
    { "table",          ParameterKind::Int,   0.0f, 251.0f,   0.0f,  0, 0 },
    { "bypass",         ParameterKind::Bool,  0.0f, 1.0f,     0.0f,  0, 0 }
}};
} // namespace

const std::array<ParameterDescriptor, 10>& parameterDescriptors()
{
    return descriptors;
}

std::string tableDisplay (int table)
{
    std::ostringstream text;
    text << std::uppercase << std::hex << std::setw (2) << std::setfill ('0') << std::clamp (table, 0, 255);
    return text.str();
}

float normalizedToSampleRate (float value)
{
    if (! std::isfinite (value))
        value = 1.0f;
    value = std::clamp (value, 0.0f, 1.0f);
    return 280.0f * std::pow (58000.0f / 280.0f, value);
}

float sampleRateToNormalized (float hz)
{
    if (! std::isfinite (hz))
        return 1.0f;
    hz = std::clamp (hz, 280.0f, 58000.0f);
    return std::clamp (std::log (hz / 280.0f) / std::log (58000.0f / 280.0f), 0.0f, 1.0f);
}

int bitControlToBits (float value)
{
    if (! std::isfinite (value))
        value = 1.0f;
    value = std::clamp (value, 0.0f, 1.0f);
    return 1 + int (std::lround (value * 7.0f));
}

int bitControlToMask (float value)
{
    if (! std::isfinite (value))
        value = 1.0f;
    value = std::clamp (value, 0.0f, 1.0f);
    return int (std::lround (value * 255.0f));
}

std::string bitDisplay (float value, bool maskMode)
{
    std::ostringstream text;
    if (maskMode)
        text << std::setw (3) << std::setfill ('0') << bitControlToMask (value);
    else
        text << bitControlToBits (value) << " bit";
    return text.str();
}

std::string sampleRateDisplay (float hz)
{
    if (! std::isfinite (hz))
        hz = 58000.0f;
    hz = std::clamp (hz, 280.0f, 58000.0f);
    std::ostringstream text;
    text << std::fixed << std::setprecision (hz < 1000.0f ? 1 : 2);
    if (hz < 1000.0f)
        text << hz << " Hz";
    else
        text << (hz / 1000.0f) << " kHz";
    return text.str();
}
} // namespace faultcounter
