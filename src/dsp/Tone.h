#pragma once

namespace faultcounter
{
class Tone
{
public:
    void prepare (double sampleRate);
    void setTone (float normalizedTone, bool enabled);
    float process (float input);

private:
    double sampleRate_ = 48000.0;
    float tone_ = 0.5f;
    bool enabled_ = true;
};
} // namespace faultcounter
