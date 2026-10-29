# Pre-mortem round — amendment 3 (S-008 UI, 2026-10-30T07:55Z)

## Scenario reviewed
Frozen T-018 amended (defect 10) + two impl-side fixes; suite re-green at
25/25; UI snapshots captured as S-008 evidence.

## Risks identified
1. `syncDisplayFromApvts()` now pushes raw values into attached controls on a
   30 Hz timer. While a user drags a slider, the parameter and raw adapter
   stay in sync via the attachment, so the push is a no-op echo — but if a
   future parameter ever desynchronizes adapter from parameter, the timer
   would fight the user's drag. Mitigation: keep the sample-rate knob's
   mouse-down guard pattern in mind if more custom-mapped knobs are added.
2. Snapshot PNGs are environment-sensitive (fonts, anti-aliasing); their
   hashes are recorded as evidence-of-render, not as golden references.
3. Button labels show the parameter name as text ("toneEnabled"); cosmetic,
   but a packaging-pass item for brand-neutral wording.

## Verdict
Acceptable to close S-008; items 2-3 tracked for S-PACKAGE.
