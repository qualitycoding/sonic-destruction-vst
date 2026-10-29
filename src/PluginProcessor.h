#pragma once

#include "dsp/Engine.h"

#include <juce_audio_processors_headless/juce_audio_processors_headless.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>

namespace faultcounter
{
class FaultCounterProcessor : public juce::AudioProcessor,
                              private juce::AudioProcessorValueTreeState::Listener
{
public:
    FaultCounterProcessor();
    ~FaultCounterProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Fault Counter"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& apvts() { return apvts_; }
    const Engine& engine() const { return engine_; }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout (std::atomic<int>& currentBitMode);

    void parameterChanged (const juce::String& parameterID, float newValue) override;

    void pullSettings();

    juce::AudioProcessorValueTreeState apvts_;
    Engine engine_;
    double hostSampleRate_ = 48000.0;
    int hostBlockSize_ = 512;
    std::atomic<int> currentBitMode_ { 0 };

    juce::LinearSmoothedValue<float> gainSmoothed_;
    juce::LinearSmoothedValue<float> toneSmoothed_;
    juce::LinearSmoothedValue<float> sampleRateSmoothed_;

    EngineSettings settings_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FaultCounterProcessor)
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();
} // namespace faultcounter
