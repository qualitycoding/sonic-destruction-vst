#include "dsp/DcBlocker.h"
#include <stdexcept>

namespace faultcounter
{
void DcBlocker::prepare (double) { throw std::logic_error ("not implemented"); }
void DcBlocker::reset() { throw std::logic_error ("not implemented"); }
float DcBlocker::process (float) { throw std::logic_error ("not implemented"); }
float DcBlocker::state() const noexcept { return previousOutput_; }
} // namespace faultcounter
