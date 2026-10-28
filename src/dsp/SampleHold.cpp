#include "dsp/SampleHold.h"
#include <stdexcept>

namespace faultcounter
{
void SampleHold::prepare (double) { throw std::logic_error ("not implemented"); }
void SampleHold::reset() { throw std::logic_error ("not implemented"); }
float SampleHold::process (float, double) { throw std::logic_error ("not implemented"); }
double SampleHold::phase() const noexcept { return phase_; }
float SampleHold::heldValue() const noexcept { return held_; }
} // namespace faultcounter
