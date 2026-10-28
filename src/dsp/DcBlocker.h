#pragma once

namespace faultcounter
{
class DcBlocker
{
public:
    void prepare (double sampleRate);
    void reset();
    float process (float input);
    float state() const noexcept;

private:
    double sampleRate_ = 48000.0;
    float previousInput_ = 0.0f;
    float previousOutput_ = 0.0f;
    float coefficient_ = 0.0f;
};
} // namespace faultcounter
