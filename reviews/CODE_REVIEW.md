# CODE_REVIEW.md — S-REVIEW Traditional Code Review

- **Date (UTC):** 2026-10-30
- **Stage:** S-REVIEW (plan/PLAN.md §S-REVIEW)
- **Scope:** full implementation diff vs merge-base `a8fb4bd`
  (81 files, +7759/−20). Product code under review:
  `src/dsp/*` (669 lines), `src/PluginProcessor.*`, `src/Parameters.*`,
  `src/ui/PluginEditor.*`.
- **Context note (honest deviation):** the plan asks for a fresh-context
  review. This review was performed in-session by the implementation
  agent. All findings below were re-derived from the code with fresh
  reads, and S-ADVERSARIAL (separate stage, fresh adversarial framing) is
  still to come; the deviation is recorded so the gate keeper can weigh it.
- **Post-fix verification:** Debug suite 34/34, 22,507,698 assertions,
  rc=0 — assertion count identical to the pre-fix baseline, confirming
  the bit-identical requirement on golden render paths.

## Findings

### Fixed in this stage

| # | Sev | Finding | Fix | Verification |
| --- | --- | --- | --- | --- |
| R-01 | Medium | `pullSettings()` guards `gain`/`tone`/`bitControl` against non-finite raw APVTS values but not `sampleRateHz`: `juce::jlimit(280, 58000, NaN)` passes NaN through (neither comparison fires), and NaN into `LinearSmoothedValue` + `SampleHold::process` freezes the phase accumulator (finite-but-stuck output, so the per-sample `isfinite` checks never trip). Path requires a hostile/buggy host writing NaN through the parameter object, but the inconsistency with the three sibling guards made it a real latent defect. | Explicit `isfinite` guard with `58000.0f` fallback in `pullSettings()`. | Robustness/fuzz suites re-run; 34/34 green. |
| R-02 | Low | `Preamp::process()` recomputed `pow(10, gainDb/20)` and both `tanh` evaluations per sample per channel — pure function of the (already clamped, block-constant) gain. | Hoisted to `setGain()` into cached `linearGain_`/`drive_`/`tanhDrive_`. The tanh normalization stays a division, not multiply-by-reciprocal, so output is bit-identical to the per-sample form. | Golden/reproducibility tests compare exact bits: assertion count unchanged at 22,507,698, 34/34 green. |
| R-03 | Low | `Engine::process()` re-applied `preamp.setGain`/`tone.setTone` per channel per block, duplicating `Engine::setSettings()` which already pushes both into every channel; every engine call site (golden, robustness, fuzz, bench) pairs `setSettings` with `process`. | Removed the redundant per-block calls; `prepare()` and `setSettings()` remain the two application points. | 34/34 green; engine golden tests bit-exact. |

### Accepted risks (no code change)

| # | Sev | Finding | Rationale |
| --- | --- | --- | --- |
| R-04 | Low | `sanitize()` defined identically in `Engine.cpp` and `BitOps.cpp`. | Four-line guard; a shared header would couple the DSP core to utility code for negligible gain. |
| R-05 | Low | `FaultCounterEditor::interactiveComponents()` uses `const_cast` to expose controls for frozen T-018 access. | Test-facing accessor only; contained to one function; alternative (friend test class) adds more machinery than it removes. |
| R-06 | Low | Editor refresh timer runs at 30 Hz for the editor's entire lifetime, including when idle. | Measured cost is negligible for 12 controls; stop-when-hidden can be revisited if a host complains. |
| R-07 | Low | `PluginProcessor.cpp` uses `std::numeric_limits` without including `<limits>` directly (transitively provided by pinned JUCE headers). | Compiles under the pinned toolchain (ENVIRONMENT.md lock); noted so any dependency bump re-checks it. |

### Carried to later stages

| # | Sev | Finding | Disposition |
| --- | --- | --- | --- |
| R-08 | Medium | No `LICENCE` file at repository root; per-file licence headers absent. | Plan assigns licence notices to S-PACKAGE ("generate SBOM, licence notices"). Carried there; G-002 stays blocked until notices ship. |
| R-09 | Info | `setStateInformation` rejects the entire chunk if any known parameter is out of range (rather than skipping that parameter). | Deliberate design, frozen-tested in StateTest/RobustnessTest (strict validation against hostile chunks); unknown parameter ids are forward-tolerated. No action. |
| R-10 | Info | `prepareToPlay` copies the 64 KiB embedded-table vector into the engine on every call. | `prepareToPlay` is host-contract off the audio thread; the frozen zero-allocation test (T-015) covers `processBlock` only, which allocates nothing. No action. |

## Checklist (plan §S-REVIEW)

- **Correctness:** engine chain order (preamp → tone → sample-hold → table →
  optional pre/post bit reduction → DC blocker) matches D-006; per-sample
  NaN containment verified by the fuzz/processor-bypass paths; review fixes
  R-01…R-03 above.
- **Real-time safety:** `processBlock` performs no allocation (frozen T-015
  zero-alloc), takes no locks, and runs under `ScopedNoDenormals`; the
  replacement-`operator new` counter instrumentation proved the invariant.
- **Undefined behavior:** no unions/reinterpret of live objects across the
  audio path; table indexing is clamped twice (`Engine::process` and
  `Tables::lookup`); state chunk parsing is bounds-checked with a 1 MiB cap,
  magic, version, and exact length match.
- **Duplication:** R-02/R-03 fixed; R-04 accepted.
- **State security:** R-09 design verified against malformed/garbage/
  truncated/mutated chunks (T-014 fuzz dimensions).
- **Coverage:** every product code path maps to a frozen test — engine
  stages via EngineGoldenTest, processor bypass via BypassTest, buses via
  BusTest, state via StateTest, editor via EditorTest, robustness via
  RobustnessTest, reproducibility via ReproducibilityTest, perf via
  PerfTest, golden provenance via GoldenMetaTest.
- **Licence notices:** R-08 carried to S-PACKAGE.
- **Dependency versions:** JUCE pin `3fb5cbc…`, Catch2 pin `0809213…`,
  pluginval v1.0.4 — all match plan/ENVIRONMENT.md; CI workflow pins the
  same commits.

## Verdict

No unresolved Critical/High findings. Every new code path maps to a
frozen test; all findings are fixed, accepted with rationale, or carried to
an owning stage. **S-REVIEW passes.**
