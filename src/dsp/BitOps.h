#pragma once

#include <cstdint>

namespace faultcounter
{
uint8_t toCode (float value);
float fromCode (uint8_t code);
uint8_t applyBits (uint8_t code, int bits);
uint8_t applyMask (uint8_t code, uint8_t mask);
} // namespace faultcounter
