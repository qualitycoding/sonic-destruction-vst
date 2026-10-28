#pragma once

namespace faultcounter
{
class Tone
{
public:
    void prepare (double sampleRate);
    void setTone (float normalizedTone, bool enabled);
    float process (float input);

public:
    struct Biquad
    {
        double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0;
        double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;
        float process (float input);
        void reset();
    };

private:
    void updateFilters();

    double sampleRate_ = 48000.0;
    float tone_ = 0.5f;
    bool enabled_ = true;
    Biquad lowPass_;
    Biquad highPass_;
};
} // namespace faultcounter
