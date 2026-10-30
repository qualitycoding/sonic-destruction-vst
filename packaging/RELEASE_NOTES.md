# Draft Release Notes — Fault Counter v0.1.0 (DRAFT, NOT RELEASED)

> **Status:** blocked at gate G-002 (see GATE-G-002.md). These notes are a
> draft for the human gate; no public release has occurred.

## Fidelity label (mandatory user-facing text)

**Fault Counter is a software emulation inspired by the WMD Geiger Counter.
It is not hardware-matched.** No physical pedal was available during
development; all transfer tables are software-generated approximations and
the sample-rate range (280 Hz–58 kHz) and tone behavior are working
conventions, not measured hardware facts. See GATE-G-001.md.

## What's inside

- VST3 effect (mono/stereo dual-mono) with: input preamp (drive),
  tilt tone stage, sample-rate reduction with Fine knob remap,
  252 8-bit wave tables (hex 00–FB), bit-depth/bitmask reduction
  (pre or post table), always-on 5 Hz DC blocker, zero-latency bypass.
- Host automation on all parameters; Fine is a UI sensitivity mode only
  (automation always addresses actual Hz).
- State save/load with strict chunk validation.

## Verification summary (evidence in repo)

- Frozen test suite: 34/34 cases, 22,507,698 assertions (Debug).
- 100,000-case hostile fuzz (NaN/Inf, malformed state, automation
  interruption): PASS, rc=0.
- Block-schedule reproducibility: bit-identical renders across irregular
  block partitions after 4096-sample settle.
- Zero audio-callback allocations (instrumented `operator new`).
- Engine benchmark: mean 44.8× realtime (20× target), 7.9% spread.
- pluginval v1.0.4 strictness 10: SUCCESS (25 sections), including on the
  rehearsed installed package.
- CI workflow: build/test matrix, ASan+UBSan fuzz, benchmark, pluginval
  matrix, dependency-pin verification (`.github/workflows/validation.yml`).
- Code review and adversarial review records: `reviews/`.

## Known limitations

- Approximation-only fidelity (hardware unavailable; see above).
- No real-DAW host matrix in the dev sandbox; pluginval covers scan/load/
  state headless, CI covers more runners.
- Windows/macOS packages must be built at release time (sandbox is
  Linux-only).
- A commercial JUCE licence is required for any closed-source release
  (A-003); the default is AGPL-3.0 source release.
