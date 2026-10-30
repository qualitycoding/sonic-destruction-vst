#pragma once

namespace faultcounter
{
class Preamp
{
public:
    Preamp();
    void setGain (float normalizedGain);
    float process (float input) const;

private:
    float normalizedGain_ = 0.2f;
    // Cached per-gain constants: identical arithmetic to the previous
    // per-sample computation (bit-identical output), hoisted off the
    // audio-thread inner loop. The tanh normalization stays a division
    // (not multiply-by-reciprocal) to remain bit-identical.
    float linearGain_ = 1.0f;
    float drive_ = 0.0f;
    float tanhDrive_ = 1.0f;
    bool cleanGain_ = true;
};
} // namespace faultcounter
