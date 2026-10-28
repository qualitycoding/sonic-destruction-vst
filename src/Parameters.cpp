#include "Parameters.h"

#include <stdexcept>

namespace faultcounter
{
const std::array<ParameterDescriptor, 9>& parameterDescriptors()
{
    throw std::logic_error ("not implemented");
}

std::string tableDisplay (int)
{
    throw std::logic_error ("not implemented");
}

float normalizedToSampleRate (float)
{
    throw std::logic_error ("not implemented");
}

float sampleRateToNormalized (float)
{
    throw std::logic_error ("not implemented");
}

int bitControlToBits (float)
{
    throw std::logic_error ("not implemented");
}

int bitControlToMask (float)
{
    throw std::logic_error ("not implemented");
}

std::string bitDisplay (float, bool)
{
    throw std::logic_error ("not implemented");
}

std::string sampleRateDisplay (float)
{
    throw std::logic_error ("not implemented");
}
} // namespace faultcounter
