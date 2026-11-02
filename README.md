# sonic-destruction-vst (Fault Counter)

A VST3 audio effect, working name **Fault Counter**, that recreates the signal flow and controls of a
"Geiger Counter"-style lo-fi distortion pedal: a high-gain preamp, an optional tone stage, sample-rate
reduction (with its intentional foldover), 1 to 8 bit depth with a Bits/Mask mode, and 252 selectable
transfer tables shown as `00` to `FB`. It does not use the original maker's name, logos, artwork or firmware.

> **Status: implemented on a separate branch, not released, and not an accurate copy.** This default branch
> (`gen-20261028T222647Z-geiger-vst3`) holds the plan, research and reviews. The code is on
> **`impl-run-20261028T222647Z-geiger-vst3`**. Release is on hold (gate G-002), and the fidelity gate (G-001) was
> decided as `approximation-only`: no hardware was available to capture, so the 252 tables are procedural
> approximations, and nothing here claims to match the original pedal.

## What the implementation branch has

- A C++ VST3 (JUCE) with Gain, Tone, Tone Enable, Sample Rate, Fine mode, Bit Depth or Mask, Bits/Mask mode,
  Pre/Post table and Bypass. All are host-automatable, saved and restored. Mono processing, with stereo handled as
  linked dual-mono.
- Tests, a 100k-iteration fuzz run (`docs/validation/fuzz-100k-summary.txt`), pluginval at strictness 10, and UI screenshots at three scales
  (`docs/validation/`, `docs/ui-evidence/`).
- A packaging rehearsal: an unsigned, Debug, Linux x86-64 build is on the `rehearsal-package-v0.1.0` branch. It was
  built before the review fixes, so it must not be distributed.

## Not done

- Windows and macOS builds: the plan targets all three desktop systems, but the rehearsal was Linux only.
- Testing in a real DAW: none was available, so loading, bypass and automation were only checked with pluginval.
- A listening test: the blind listening gate (G-LISTEN) is blocked and nothing has been scored.
- Hardware-based table capture and calibration (T-019 to T-021 never ran).
- Release. A non-commercial licence applies by default: the source is AGPL-3.0-or-later (`LICENCE.md` on the
  implementation branch). Closed-source or commercial distribution needs a commercial JUCE licence first.

## Other contents of this branch

`plan/PLAN.md` is the full plan and the best place to start. `scripts/make_blind_pairs.py` and
`scripts/score_blind.py` build and score the blind listening test. `lessons/` and `knowledge/` hold notes
produced during the run.
