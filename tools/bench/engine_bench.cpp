// SC-09 supporting evidence: engine-level throughput benchmark (no JUCE).
// The frozen T-015 test enforces the processor-level target (20x release,
// 2x Debug floor); this standalone benchmark isolates the DSP hot path at
// -O2 to demonstrate the SC-09 headroom independent of build-type quirks.
#include "dsp/Engine.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <vector>

int main (int argc, char** argv)
{
    if (argc < 2)
    {
        std::fprintf (stderr, "usage: %s <tables.bin>\n", argv[0]);
        return 2;
    }
    std::ifstream input (argv[1], std::ios::binary);
    const std::vector<uint8_t> tables ((std::istreambuf_iterator<char> (input)), std::istreambuf_iterator<char>());
    if (tables.empty())
    {
        std::fprintf (stderr, "cannot read table asset\n");
        return 2;
    }

    constexpr double hostRate = 48000.0;
    constexpr int blockSize = 64;
    constexpr int secondsPerRun = 30;
    constexpr int numBlocks = int (hostRate) / blockSize * secondsPerRun;

    faultcounter::EngineSettings settings;
    settings.gain = 0.6f;
    settings.tone = 0.5f;
    settings.toneEnabled = true;
    settings.sampleRateHz = 9000.0f;
    settings.bitControl = 0.5f;
    settings.bitMode = faultcounter::BitMode::Bits;
    settings.bitPosition = faultcounter::BitPosition::Post;
    settings.table = 42;
    settings.bypass = false;

    double factors[3];
    for (int run = 0; run < 3; ++run)
    {
        faultcounter::Engine engine;
        engine.prepare (hostRate, tables);
        engine.setSettings (settings);

        std::vector<std::vector<float>> buffers (2, std::vector<float> (size_t (blockSize)));
        float* channels[] = { buffers[0].data(), buffers[1].data() };

        const auto start = std::chrono::steady_clock::now();
        for (int b = 0; b < numBlocks; ++b)
        {
            for (int i = 0; i < blockSize; ++i)
            {
                const int n = b * blockSize + i;
                buffers[0][size_t (i)] = float (0.5 * std::sin (2.0 * 3.14159265358979323846 * 220.0 * double (n) / hostRate));
                buffers[1][size_t (i)] = buffers[0][size_t (i)];
            }
            engine.process (channels, 2, blockSize);
        }
        const double elapsed = std::chrono::duration<double> (std::chrono::steady_clock::now() - start).count();
        factors[run] = double (numBlocks) * blockSize / hostRate / elapsed;
        std::printf ("run %d: %.1fx real time (%.2f s for %d s of audio)\n", run, factors[run], elapsed, secondsPerRun);
    }
    const double mean = (factors[0] + factors[1] + factors[2]) / 3.0;
    const double hi = std::max ({ factors[0], factors[1], factors[2] });
    const double lo = std::min ({ factors[0], factors[1], factors[2] });
    const double spread = (hi - lo) / mean;
    std::printf ("mean %.1fx, spread %.1f%%\n", mean, spread * 100.0);
    return mean >= 20.0 ? 0 : 1;
}
