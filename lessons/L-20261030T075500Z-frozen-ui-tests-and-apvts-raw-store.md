# Lesson — frozen UI tests and APVTS raw-store semantics (2026-10-30, S-008)

## What happened
T-018 (frozen) failed green-run against the real editor for three distinct
reasons in sequence:

1. Sliders do not request keyboard focus by default in JUCE — a frozen
   accessibility assertion forced `setWantsKeyboardFocus(true)` per control.
2. The test hardcoded 9000 Hz as a "sub-half-scale" frequency, but the frozen
   T-001 mapping is logarithmic; 9000 Hz sits at v ≈ 0.65. The frozen test
   contradicted another frozen test (defect 10, TEST_CHALLENGE round 3).
3. The reopen case stored `gain` via `getRawParameterValue()->store()`, which
   bypasses the parameter object; APVTS attachments follow the parameter, so a
   fresh editor showed the stale value until `syncDisplayFromApvts()` pushed
   raw adapter values into the controls (impl fix, test was correct).

## Rules going forward
- When a frozen test picks numeric examples, derive them from the frozen
  mapping/helper functions instead of hardcoding raw values whose position on
  the curve depends on the mapping shape.
- Any code path that can observe APVTS state via the raw adapter (editor
  reopen, state restore, tests) must treat the adapter atomic as the source of
  truth for display, and push into view controls with `dontSendNotification`
  under a re-entry guard.
- JUCE accessibility defaults are opt-in per component: `Slider` and
  `ToggleButton` need explicit `setWantsKeyboardFocus(true)` and explicit
  focus ordering for keyboard reachability.
