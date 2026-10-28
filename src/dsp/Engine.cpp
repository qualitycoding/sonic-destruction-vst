#include "dsp/Engine.h"
#include <stdexcept>

namespace faultcounter
{
void Engine::prepare (double, const std::vector<uint8_t>&) { throw std::logic_error ("not implemented"); }
void Engine::reset() { throw std::logic_error ("not implemented"); }
void Engine::setSettings (const EngineSettings&) { throw std::logic_error ("not implemented"); }
void Engine::process (float* const*, int, int) { throw std::logic_error ("not implemented"); }
} // namespace faultcounter
