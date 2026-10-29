// S-008: accessible, brand-independent pedal-inspired editor (D-003).
#include "PluginEditor.h"

#include "Parameters.h"

namespace faultcounter
{
namespace
{
void styleRotary (juce::Slider& slider)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 18);
    slider.setWantsKeyboardFocus (true);
}

void styleToggle (juce::ToggleButton& button, const char* tooltip)
{
    button.setButtonText (button.getName());
    button.setTooltip (tooltip);
    button.setWantsKeyboardFocus (true);
}
} // namespace

FaultCounterEditor::FaultCounterEditor (FaultCounterProcessor& processor)
    : AudioProcessorEditor (&processor),
      processor_ (processor),
      apvts_ (processor.apvts())
{
    setUiScale (1.0f);

    titleLabel_.setJustificationType (juce::Justification::centredLeft);
    titleLabel_.setFont (juce::Font (22.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel_);

    hexDisplay_.setJustificationType (juce::Justification::centred);
    hexDisplay_.setFont (juce::Font (36.0f, juce::Font::bold));
    hexDisplay_.setTooltip ("Wave table, hexadecimal");
    addAndMakeVisible (hexDisplay_);

    styleRotary (gainSlider_);
    gainSlider_.setTooltip ("Input gain");
    addAndMakeVisible (gainSlider_);

    styleRotary (toneSlider_);
    toneSlider_.setTooltip ("Tone");
    addAndMakeVisible (toneSlider_);

    styleRotary (sampleRateKnob_);
    sampleRateKnob_.setTooltip ("Sample rate; Fine halves the knob range without changing Hz");
    sampleRateKnob_.onValueChange = [this] { applySampleRateFromKnob(); };
    sampleRateKnob_.onDragStart = [this] {
        if (auto* parameter = apvts_.getParameter ("sampleRate"))
            parameter->beginChangeGesture();
    };
    sampleRateKnob_.onDragEnd = [this] {
        if (auto* parameter = apvts_.getParameter ("sampleRate"))
            parameter->endChangeGesture();
    };
    addAndMakeVisible (sampleRateKnob_);

    styleRotary (bitControlSlider_);
    bitControlSlider_.setTooltip ("Bit depth (Bits) or bitmask (Mask)");
    addAndMakeVisible (bitControlSlider_);

    styleRotary (tableSlider_);
    tableSlider_.setTooltip ("Wave table selector, 00…FB");
    addAndMakeVisible (tableSlider_);

    styleToggle (toneEnabledButton_, "Enable the tone stage");
    styleToggle (fineButton_, "Fine sample-rate adjustment (upper half of the range)");
    styleToggle (bitModeButton_, "Mask mode when lit, Bits mode when dark");
    styleToggle (bitPositionButton_, "Pre when lit, Post when dark");
    styleToggle (bypassButton_, "Exact zero-latency bypass");
    for (auto* button : { &toneEnabledButton_, &fineButton_, &bitModeButton_, &bitPositionButton_, &bypassButton_ })
    {
        addAndMakeVisible (*button);
        button->addListener (this);
    }

    for (auto* label : { &gainLabel_, &toneLabel_, &rateLabel_, &bitLabel_, &tableLabel_ })
    {
        label->setJustificationType (juce::Justification::centred);
        addAndMakeVisible (*label);
    }

    gainAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts_, "gain", gainSlider_);
    toneAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts_, "tone", toneSlider_);
    bitControlAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts_, "bitControl", bitControlSlider_);
    tableAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts_, "table", tableSlider_);
    toneEnabledAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts_, "toneEnabled", toneEnabledButton_);
    fineAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts_, "fine", fineButton_);
    bitModeAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts_, "bitMode", bitModeButton_);
    bitPositionAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts_, "bitPosition", bitPositionButton_);
    bypassAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts_, "bypass", bypassButton_);

    syncDisplayFromApvts();
    startTimerHz (30);
}

FaultCounterEditor::~FaultCounterEditor()
{
    stopTimer();
}

void FaultCounterEditor::setUiScale (float scale)
{
    scale_ = juce::jlimit (0.5f, 3.0f, scale);
    setSize (int (float (baseWidth) * scale_), int (float (baseHeight) * scale_));
}

void FaultCounterEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2a2d31), bounds.getCentreX(), 0.0f,
                                             juce::Colour (0xff17181a), bounds.getCentreX(), bounds.getBottom(), false));
    g.fillAll();
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.drawRect (getLocalBounds().reduced (int (4.0f * scale_)), int (2.0f * scale_));
}

void FaultCounterEditor::resized()
{
    const auto s = scale_;
    auto area = getLocalBounds().reduced (int (14.0f * s));

    auto header = area.removeFromTop (int (40.0f * s));
    titleLabel_.setBounds (header.removeFromLeft (int (300.0f * s)));
    hexDisplay_.setBounds (header.removeFromRight (int (110.0f * s)));

    auto toggles = area.removeFromBottom (int (34.0f * s));
    const int toggleWidth = int (112.0f * s);
    for (auto* button : { &toneEnabledButton_, &fineButton_, &bitModeButton_, &bitPositionButton_, &bypassButton_ })
        button->setBounds (toggles.removeFromLeft (toggleWidth).reduced (int (3.0f * s), int (4.0f * s)));

    // Knob row: each knob column carries its caption above the rotary.
    const int columns = 5;
    const int columnWidth = area.getWidth() / columns;
    struct Column
    {
        juce::Label* label;
        juce::Slider* slider;
    };
    const Column layout[] = {
        { &gainLabel_, &gainSlider_ },
        { &toneLabel_, &toneSlider_ },
        { &rateLabel_, &sampleRateKnob_ },
        { &bitLabel_, &bitControlSlider_ },
        { &tableLabel_, &tableSlider_ },
    };
    for (int i = 0; i < columns; ++i)
    {
        auto column = area.removeFromLeft (columnWidth).reduced (int (6.0f * s));
        layout[i].label->setBounds (column.removeFromTop (int (18.0f * s)));
        layout[i].slider->setBounds (column);
    }

    // Keyboard focus order follows visual layout order.
    int order = 1;
    for (auto* control : interactiveComponents())
        control->setExplicitFocusOrder (order++);
}

std::vector<juce::Component*> FaultCounterEditor::interactiveComponents() const
{
    // Visual order == focus order.
    return { const_cast<juce::Slider*> (&gainSlider_),   const_cast<juce::Slider*> (&toneSlider_),
             const_cast<juce::Slider*> (&sampleRateKnob_), const_cast<juce::Slider*> (&bitControlSlider_),
             const_cast<juce::Slider*> (&tableSlider_),  const_cast<juce::ToggleButton*> (&toneEnabledButton_),
             const_cast<juce::ToggleButton*> (&fineButton_), const_cast<juce::ToggleButton*> (&bitModeButton_),
             const_cast<juce::ToggleButton*> (&bitPositionButton_), const_cast<juce::ToggleButton*> (&bypassButton_) };
}

float FaultCounterEditor::knobValueFromHz() const
{
    const auto hz = apvts_.getRawParameterValue ("sampleRate")->load();
    const auto v = sampleRateToNormalized (hz);
    if (! fineButton_.getToggleState())
        return v;
    return v >= 0.5f ? 2.0f * v - 1.0f : 0.0f;
}

float FaultCounterEditor::hzFromKnobValue (float knobValue) const
{
    if (! fineButton_.getToggleState())
        return normalizedToSampleRate (knobValue);
    return normalizedToSampleRate (0.5f + 0.5f * knobValue);
}

void FaultCounterEditor::applySampleRateFromKnob()
{
    if (updatingFromApvts_)
        return;
    const auto hz = hzFromKnobValue (float (sampleRateKnob_.getValue()));
    if (auto* parameter = apvts_.getParameter ("sampleRate"))
        parameter->setValueNotifyingHost (parameter->convertTo0to1 (hz));
}

void FaultCounterEditor::syncDisplayFromApvts()
{
    const auto table = int (std::lround (apvts_.getRawParameterValue ("table")->load()));
    hexDisplay_.setText (juce::String (tableDisplay (table)), juce::dontSendNotification);

    // Attached controls follow the raw APVTS adapter values. Hosts normally
    // drive these through the parameter objects, but raw-value stores (and a
    // fresh editor reopen) bypass the attachments, so push explicitly.
    // dontSendNotification keeps the attachments from echoing back into the
    // parameters.
    updatingFromApvts_ = true;
    gainSlider_.setValue (apvts_.getRawParameterValue ("gain")->load(), juce::dontSendNotification);
    toneSlider_.setValue (apvts_.getRawParameterValue ("tone")->load(), juce::dontSendNotification);
    bitControlSlider_.setValue (apvts_.getRawParameterValue ("bitControl")->load(), juce::dontSendNotification);
    tableSlider_.setValue (apvts_.getRawParameterValue ("table")->load(), juce::dontSendNotification);
    const auto toggleFromRaw = [this] (const char* id, juce::ToggleButton& button) {
        button.setToggleState (apvts_.getRawParameterValue (id)->load() >= 0.5f, juce::dontSendNotification);
    };
    toggleFromRaw ("toneEnabled", toneEnabledButton_);
    toggleFromRaw ("fine", fineButton_);
    toggleFromRaw ("bitMode", bitModeButton_);
    toggleFromRaw ("bitPosition", bitPositionButton_);
    toggleFromRaw ("bypass", bypassButton_);
    updatingFromApvts_ = false;

    const auto knobValue = knobValueFromHz();
    if (! sampleRateKnob_.isMouseButtonDown() && ! sampleRateKnob_.isMouseOverOrDragging()
        && ! juce::approximatelyEqual (float (sampleRateKnob_.getValue()), knobValue))
    {
        updatingFromApvts_ = true;
        sampleRateKnob_.setValue (knobValue, juce::dontSendNotification);
        updatingFromApvts_ = false;
    }
}

void FaultCounterEditor::buttonStateChanged (juce::Button* button)
{
    if (button == &fineButton_)
    {
        // D-003: switching Fine preserves the audible Hz value; the knob
        // remaps and, for v < 0.5, shows u = 0 until the user moves it.
        updatingFromApvts_ = true;
        sampleRateKnob_.setValue (knobValueFromHz(), juce::dontSendNotification);
        updatingFromApvts_ = false;
    }
}
} // namespace faultcounter
