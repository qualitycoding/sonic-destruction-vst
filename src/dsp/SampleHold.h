#pragma once

namespace faultcounter
{
class SampleHold
{
public:
    void prepare (double sampleRate);
    void reset();
    float process (float input, double requestedRate);
    double phase() const noexcept;
    float heldValue() const noexcept;

private:
    double hostRate_ = 48000.0;
    double phase_ = 1.0;
    float held_ = 0.0f;
};
} // namespace faultcounter
