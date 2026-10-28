#pragma once

namespace faultcounter
{
class Preamp
{
public:
    void setGain (float normalizedGain);
    float process (float input) const;

private:
    float normalizedGain_ = 0.2f;
};
} // namespace faultcounter
