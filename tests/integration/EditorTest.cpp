// T-018: at 100%, 150%, and 200% UI scale every control is visible, labeled,
// keyboard reachable, and matches APVTS after edit/reopen.
#include "Parameters.h"
#include "PluginProcessor.h"
#include "ui/PluginEditor.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>
#include <cstdlib>
#include <set>
#include <vector>

using namespace faultcounter;
using Catch::Matchers::WithinAbs;

namespace
{
std::vector<juce::Component*> collectFocusCycle (juce::Component& root)
{
    std::vector<juce::Component*> cycle;
    auto traverser = root.createFocusTraverser();
    auto* first = traverser->getDefaultComponent (&root);
    if (first == nullptr)
        return cycle;

    cycle.push_back (first);
    for (auto* current = first; auto* next = traverser->getNextComponent (current);)
    {
        if (next == first)
            break;
        cycle.push_back (next);
        current = next;
        if (cycle.size() > 64) // runaway guard
            break;
    }
    return cycle;
}

void requireControlUsable (FaultCounterEditor& editor, juce::Component& control, float scale, const char* name)
{
    CAPTURE (name, scale);
    // Visible inside the editor at this scale.
    REQUIRE (editor.getLocalBounds().contains (control.getBounds()));
    REQUIRE (control.getWidth() >= int (18.0f * scale));
    REQUIRE (control.getHeight() >= int (18.0f * scale));
    // Labeled: every interactive control carries a name.
    REQUIRE (control.getName().isNotEmpty());
    // Keyboard focusable.
    REQUIRE (control.getWantsKeyboardFocus());
}
} // namespace

TEST_CASE ("T-018 controls usable at 100/150/200 percent", "[frozen][ui]")
{
    FaultCounterProcessor processor;
    std::unique_ptr<FaultCounterEditor> editor (dynamic_cast<FaultCounterEditor*> (processor.createEditor()));
    REQUIRE (editor != nullptr);

    for (const float scale : { 1.0f, 1.5f, 2.0f })
    {
        CAPTURE (scale);
        editor->setUiScale (scale);
        editor->resized();

        const auto& controls = editor->interactiveComponents();
        REQUIRE (controls.size() >= 10);
        for (auto* control : controls)
            requireControlUsable (*editor, *control, scale,
                                  control->getName().isNotEmpty() ? control->getName().toRawUTF8()
                                                                  : "control");

        // Keyboard reachability: the focus-traversal cycle must cover every
        // interactive control at this scale.
        const auto cycle = collectFocusCycle (*editor);
        std::set<juce::Component*> reachable (cycle.begin(), cycle.end());
        for (auto* control : controls)
        {
            CAPTURE (scale, control->getName().toRawUTF8());
            REQUIRE (reachable.count (control) == 1);
        }
    }
}

TEST_CASE ("T-018 edits synchronize to APVTS and reopen reflects state", "[frozen][ui]")
{
    FaultCounterProcessor processor;
    {
        std::unique_ptr<FaultCounterEditor> editor (dynamic_cast<FaultCounterEditor*> (processor.createEditor()));
        REQUIRE (editor != nullptr);
        editor->setUiScale (1.0f);

        auto& state = processor.apvts();

        editor->gainSlider().setValue (0.8, juce::sendNotificationSync);
        REQUIRE_THAT (double (state.getRawParameterValue ("gain")->load()), WithinAbs (0.8, 1e-5));

        editor->bypassButton().setToggleState (true, juce::sendNotificationSync);
        REQUIRE (state.getRawParameterValue ("bypass")->load() == 1.0f);

        editor->bitModeButton().setToggleState (true, juce::sendNotificationSync);
        REQUIRE (state.getRawParameterValue ("bitMode")->load() == 1.0f);

        editor->tableSlider().setValue (42.0, juce::sendNotificationSync);
        REQUIRE (state.getRawParameterValue ("table")->load() == 42.0f);
        editor->syncDisplayFromApvts();
        REQUIRE (editor->tableHexLabel().getText() == "2A");

        // Fine behaviour (D-003): enabling Fine with v < 0.5 shows u = 0 and
        // leaves Hz unchanged; moving the knob maps v = 0.5 + 0.5u.
        // The D-003 mapping is logarithmic (frozen T-001), so derive the
        // sub-half-scale frequency from a normalized target instead of
        // hardcoding 9000 Hz (which lands above 0.5 on the log scale).
        const float hzLow = normalizedToSampleRate (0.4f);
        state.getRawParameterValue ("sampleRate")->store (hzLow);
        editor->syncDisplayFromApvts();
        const float vLow = sampleRateToNormalized (hzLow);
        REQUIRE (vLow < 0.5f);
        editor->fineButton().setToggleState (true, juce::sendNotificationSync);
        REQUIRE_THAT (double (editor->sampleRateKnob().getValue()), WithinAbs (0.0, 1e-6));
        REQUIRE_THAT (double (state.getRawParameterValue ("sampleRate")->load()), WithinAbs (double (hzLow), 1e-1));

        editor->sampleRateKnob().setValue (0.5, juce::sendNotificationSync);
        const float expectedHz = normalizedToSampleRate (0.5f + 0.5f * 0.5f);
        REQUIRE_THAT (double (state.getRawParameterValue ("sampleRate")->load()), WithinAbs (double (expectedHz), 1e-1));

        editor->fineButton().setToggleState (false, juce::sendNotificationSync);
        REQUIRE_THAT (double (editor->sampleRateKnob().getValue()),
                       WithinAbs (double (sampleRateToNormalized (expectedHz)), 1e-5));
    }

    // Reopen: a fresh editor reflects the APVTS state (host synchronization).
    processor.apvts().getRawParameterValue ("gain")->store (0.33f);
    processor.apvts().getRawParameterValue ("table")->store (251.0f);
    std::unique_ptr<FaultCounterEditor> reopened (dynamic_cast<FaultCounterEditor*> (processor.createEditor()));
    REQUIRE (reopened != nullptr);
    for (const float scale : { 1.0f, 1.5f, 2.0f })
    {
        CAPTURE (scale);
        reopened->setUiScale (scale);
        reopened->syncDisplayFromApvts();
        REQUIRE_THAT (double (reopened->gainSlider().getValue()), WithinAbs (0.33, 1e-5));
        REQUIRE (reopened->tableHexLabel().getText() == "FB");
    }
}

TEST_CASE ("T-018 UI snapshot capture (evidence, env-gated)", "[frozen][ui]")
{
    // Assertions run unconditionally; PNG evidence is written only when
    // FAULTCOUNTER_UI_OUT names an existing directory (capture runs).
    FaultCounterProcessor processor;
    std::unique_ptr<FaultCounterEditor> editor (dynamic_cast<FaultCounterEditor*> (processor.createEditor()));
    REQUIRE (editor != nullptr);

    if (const char* outDir = std::getenv ("FAULTCOUNTER_UI_OUT"))
    {
        for (const float scale : { 1.0f, 1.5f, 2.0f })
        {
            editor->setUiScale (scale);
            const auto image = editor->createComponentSnapshot (editor->getLocalBounds(), scale);
            const auto fileName = juce::String (outDir) + "/ui-" + juce::String (int (scale * 100.0f)) + ".png";
            REQUIRE (image.isValid());
            REQUIRE (juce::PNGImageFormat().writeImageToStream (image, *juce::File (fileName).createOutputStream().get()));
        }
    }

    REQUIRE (editor->getWidth() > 0);
}
