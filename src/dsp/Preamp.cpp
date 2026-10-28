#include "dsp/Preamp.h"
#include <stdexcept>

namespace faultcounter
{
void Preamp::setGain (float) { throw std::logic_error ("not implemented"); }
float Preamp::process (float) const { throw std::logic_error ("not implemented"); }
} // namespace faultcounter
