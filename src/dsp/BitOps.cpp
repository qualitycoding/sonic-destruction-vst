#include "dsp/BitOps.h"
#include <stdexcept>

namespace faultcounter
{
uint8_t toCode (float) { throw std::logic_error ("not implemented"); }
float fromCode (uint8_t) { throw std::logic_error ("not implemented"); }
uint8_t applyBits (uint8_t, int) { throw std::logic_error ("not implemented"); }
uint8_t applyMask (uint8_t, uint8_t) { throw std::logic_error ("not implemented"); }
} // namespace faultcounter
