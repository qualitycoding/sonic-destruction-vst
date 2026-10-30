// T-015: real-time safety and throughput — zero allocations while counting
// is armed during processBlock, and the SC-09 benchmark sustains the required
// real-time factor on three repeated runs with <= 10% relative spread.
#include "PluginProcessor.h"

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <new>
#include <vector>

using namespace faultcounter;

// Allocation instrumentation local to the test binary. The audio thread in
// this test is the calling thread; arming the counter around processBlock
// turns any heap traffic from the plug-in into a test failure. Replacement
// global operators must live at global scope (not in an anonymous namespace).
std::atomic<long> gAllocations { 0 };
std::atomic<bool> gCounting { false };

void* operator new (std::size_t size)
{
    if (gCounting.load (std::memory_order_relaxed))
        gAllocations.fetch_add (1, std::memory_order_relaxed);
    void* ptr = std::malloc (size == 0 ? 1 : size);
    if (ptr == nullptr)
        throw std::bad_alloc();
    return ptr;
}

void operator delete (void* ptr) noexcept
{
    std::free (ptr);
}

void operator delete (void* ptr, std::size_t) noexcept
{
    std::free (ptr);
}

namespace
{
void fillSteady (juce::AudioBuffer<float>& buffer, int sampleOffset)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* data = buffer.getWritePointer (channel);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const int n = sampleOffset + i;
            data[i] = float (0.5 * std::sin (2.0 * 3.14159265358979323846 * 220.0 * double (n) / 48000.0));
        }
    }
}

void setSteadyParameters (FaultCounterProcessor& processor)
{
    auto& state = processor.apvts();
    state.getRawParameterValue ("gain")->store (0.6f);
    state.getRawParameterValue ("tone")->store (0.5f);
    state.getRawParameterValue ("toneEnabled")->store (1.0f);
    state.getRawParameterValue ("sampleRate")->store (9000.0f);
    state.getRawParameterValue ("fine")->store (0.0f);
    state.getRawParameterValue ("bitControl")->store (0.5f);
    state.getRawParameterValue ("bitMode")->store (0.0f);
    state.getRawParameterValue ("bitPosition")->store (0.0f);
    state.getRawParameterValue ("table")->store (42.0f);
    state.getRawParameterValue ("bypass")->store (0.0f);
}

// SC-09 fixes the target at the release build; Debug builds use a smoke
// floor so this frozen test stays meaningful on every configuration.
#ifdef NDEBUG
constexpr double requiredRealtimeFactor = 20.0;
#else
constexpr double requiredRealtimeFactor = 2.0;
#endif
} // namespace

TEST_CASE ("T-015 audio callback performs zero allocations while armed", "[frozen][performance]")
{
    FaultCounterProcessor processor;
    setSteadyParameters (processor);
    processor.prepareToPlay (48000.0, 64);

    juce::AudioBuffer<float> buffer (2, 64);
    juce::MidiBuffer midi;
    int sampleOffset = 0;
    auto settle = [&] {
        for (int b = 0; b < 64; ++b)
        {
            fillSteady (buffer, sampleOffset);
            sampleOffset += buffer.getNumSamples();
            processor.processBlock (buffer, midi);
        }
    };
    settle();

    gAllocations.store (0, std::memory_order_relaxed);
    gCounting.store (true, std::memory_order_relaxed);
    settle();
    gCounting.store (false, std::memory_order_relaxed);

    REQUIRE (gAllocations.load (std::memory_order_relaxed) == 0);
}

TEST_CASE ("T-015 SC-09 benchmark sustains real-time factor with low variance", "[frozen][performance]")
{
    constexpr int blockSize = 64;
    constexpr double hostRate = 48000.0;
    constexpr int secondsPerRun = 10;
    constexpr int blocksPerRun = int (hostRate) / blockSize * secondsPerRun;

    double results[3];
    for (int run = 0; run < 3; ++run)
    {
        FaultCounterProcessor processor;
        setSteadyParameters (processor);
        processor.prepareToPlay (hostRate, blockSize);

        juce::AudioBuffer<float> buffer (2, blockSize);
        juce::MidiBuffer midi;
        int sampleOffset = 0;
        for (int b = 0; b < blocksPerRun; ++b)
        {
            fillSteady (buffer, sampleOffset);
            sampleOffset += blockSize;
            processor.processBlock (buffer, midi);
        }

        // Measure a second pass over identically-prepared steady state so the
        // timing loop excludes parameter smoothing and cache warm-up.
        const auto start = std::chrono::steady_clock::now();
        for (int b = 0; b < blocksPerRun; ++b)
            processor.processBlock (buffer, midi);
        const auto elapsed = std::chrono::duration<double> (std::chrono::steady_clock::now() - start).count();

        results[run] = double (blocksPerRun) * blockSize / hostRate / elapsed;
        CAPTURE (run, results[run]);
    }

    const double mean = (results[0] + results[1] + results[2]) / 3.0;
    const double spread = (std::max ({ results[0], results[1], results[2] })
                           - std::min ({ results[0], results[1], results[2] }))
                          / mean;
    CAPTURE (mean, spread, requiredRealtimeFactor);
    REQUIRE (mean >= requiredRealtimeFactor);
    REQUIRE (spread <= 0.10);
}
