#include "dsp/Tone.h"
#include <stdexcept>

namespace faultcounter
{
void Tone::prepare (double) { throw std::logic_error ("not implemented"); }
void Tone::setTone (float, bool) { throw std::logic_error ("not implemented"); }
float Tone::process (float) { throw std::logic_error ("not implemented"); }
} // namespace faultcounter
