// S-008: accessible, brand-independent pedal-inspired editor (D-003).
#pragma once

#include "../PluginProcessor.h"

#include <juce_gui_extra/juce_gui_extra.h>

#include <memory>
#include <vector>

namespace faultcounter
{
class FaultCounterEditor : public juce::AudioProcessorEditor,
                           private juce::Timer,
                           private juce::Button::Listener
{
public:
    explicit FaultCounterEditor (FaultCounterProcessor&);
    ~FaultCounterEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // 1.0 / 1.5 / 2.0 (any positive value accepted; D-003 scaling evidence).
    void setUiScale (float scale);
    float uiScale() const { return scale_; }

    // Re-reads APVTS into display-only and custom-mapped controls. Also called
    // by the refresh timer; public so tests can force synchronization.
    void syncDisplayFromApvts();

    // T-018 accessors.
    std::vector<juce::Component*> interactiveComponents() const;
    juce::Slider& gainSlider() { return gainSlider_; }
    juce::Slider& sampleRateKnob() { return sampleRateKnob_; }
    juce::Slider& tableSlider() { return tableSlider_; }
    juce::ToggleButton& fineButton() { return fineButton_; }
    juce::ToggleButton& bitModeButton() { return bitModeButton_; }
    juce::ToggleButton& bypassButton() { return bypassButton_; }
    juce::Label& tableHexLabel() { return hexDisplay_; }

private:
    static constexpr int baseWidth = 620;
    static constexpr int baseHeight = 360;

    void timerCallback() override { syncDisplayFromApvts(); }
    void buttonClicked (juce::Button*) override {}
    void buttonStateChanged (juce::Button*) override;

    float knobValueFromHz() const;
    float hzFromKnobValue (float knobValue) const;
    void applySampleRateFromKnob();

    FaultCounterProcessor& processor_;
    juce::AudioProcessorValueTreeState& apvts_;
    float scale_ = 1.0f;
    bool updatingFromApvts_ = false;

    juce::Label titleLabel_ { "title", "FAULT COUNTER" };
    juce::Label hexDisplay_ { "hexDisplay", "00" };

    juce::Slider gainSlider_ { "gain" };
    juce::Slider toneSlider_ { "tone" };
    juce::Slider sampleRateKnob_ { "sampleRate" };
    juce::Slider bitControlSlider_ { "bitControl" };
    juce::Slider tableSlider_ { "table" };

    juce::ToggleButton toneEnabledButton_ { "toneEnabled" };
    juce::ToggleButton fineButton_ { "fine" };
    juce::ToggleButton bitModeButton_ { "bitMode" };
    juce::ToggleButton bitPositionButton_ { "bitPosition" };
    juce::ToggleButton bypassButton_ { "bypass" };

    juce::Label gainLabel_ { "gainLabel", "GAIN" };
    juce::Label toneLabel_ { "toneLabel", "TONE" };
    juce::Label rateLabel_ { "rateLabel", "SAMPLE RATE" };
    juce::Label bitLabel_ { "bitLabel", "BITS / MASK" };
    juce::Label tableLabel_ { "tableLabel", "TABLE" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> toneAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bitControlAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tableAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> toneEnabledAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> fineAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bitModeAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bitPositionAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FaultCounterEditor)
};
} // namespace faultcounter
