#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace faultcounter
{
enum class ParameterKind { Float, Bool, Choice, Int };

struct ParameterDescriptor
{
    std::string_view id;
    ParameterKind kind;
    float minimum;
    float maximum;
    float defaultValue;
    int defaultChoice;
    int choiceCount;
};

inline constexpr int tableCount = 252;

const std::array<ParameterDescriptor, 9>& parameterDescriptors();
std::string tableDisplay (int table);
float normalizedToSampleRate (float value);
float sampleRateToNormalized (float hz);
int bitControlToBits (float value);
int bitControlToMask (float value);
std::string bitDisplay (float value, bool maskMode);
std::string sampleRateDisplay (float hz);
} // namespace faultcounter
