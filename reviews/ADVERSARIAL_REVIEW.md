# ADVERSARIAL_REVIEW.md — S-ADVERSARIAL

- **Date (UTC):** 2026-10-30
- **Stage:** S-ADVERSARIAL (plan/PLAN.md §S-ADVERSARIAL)
- **Context note:** same in-session-context deviation as recorded in
  `reviews/CODE_REVIEW.md`; the adversarial framing below is applied to
  assumptions, hostile inputs, and overclaim vectors regardless.

## Adversarial probes performed

1. **Assumption challenges (A-001…A-010).**
   - A-001/A-002 (standard pedal, VST3 only): no code path builds or claims
     another format; `FORMATS VST3` only.
   - A-003 (JUCE licence): carried — closed-source release requires a
     commercial JUCE licence before G-002; recorded in GATE/S-PACKAGE scope.
   - A-004 (hardware absent): `research/HARDWARE_UNAVAILABLE.md` +
     `GATE-G-001.md` keep the approximation label attached; verified no
     code, table, or doc fabricates capture evidence.
   - A-005 (automation replaces CV): no MIDI/CV invention in code.
   - A-006 (stereo dual-mono): channels share parameters via one
     `EngineSettings`; per-channel state isolated — a burst on one channel
     cannot desynchronize the other beyond the shared block-granular
     settings (by design, frozen-tested in T-013/T-016).
   - A-007/A-008 (range/Fine conventions): still conventions; labeled as
     such; the review confirms the DSP treats Fine as a UI-only remap
     (editor-only code path) and host automation always addresses Hz.
   - A-009 (5 Hz DC blocker): coefficient `1 − 2π·5/sr` is stable
     (|pole| = coefficient < 1 for all sr ≥ 1); cannot hide instability
     because engine output is additionally bounded by the 8-bit table
     reconstruction (|out| ≤ 1) — the blocker cannot amplify.
   - A-010: see plan table; no contradiction found in code.

2. **Malformed state (beyond frozen coverage):** 12 MiB chunk → rejected
   by `kMaxStateChunkBytes` (1 MiB) before any parse; length-field
   mismatch → rejected; unknown parameter ids → forward-tolerated; known
   id out of range → whole-chunk reject (deliberate, R-09). No parser
   crash vector found in the capped XML surface.

3. **Hostile parameter sequences:** `R-01` (S-REVIEW) already closed the
   NaN-parameter path into `sampleRateHz` (smoother freeze). Re-probed the
   sibling paths after the fix: NaN normalised value through
   `convertTo0to1` → raw NaN → now falls back to 58000 Hz; `bitMode` NaN →
   `lround(NaN)` is UB in theory — audited: `bitMode` only reaches
   `parameterChanged` from APVTS value commits, and APVTS choice values are
   always 0/1 from the parameter object; raw-adapter stores of arbitrary
   floats into a `choice` parameter are host-contract violations outside
   the validated surface (state chunks clamp to 0/1 via `knownParamRange`).
   Accepted risk, no reachable path found.

4. **Denormals/NaN/Inf:** processor path runs under `ScopedNoDenormals`;
   every non-finite sample is sanitized at entry (`Engine::process`) and
   every output is re-checked per sample; the bypass path zeroes non-finite
   samples (SC-06 fix from S-009). DC-blocker pole proximity to 1.0 cannot
   produce runaway: output is the difference equation on bounded input.

5. **Block-schedule variation:** frozen T-016 proves bit-invariance after
   a 4096-sample settle across `{64,1,511,2,1024,7,4096,16,10000,663}`;
   T-013 sweeps SC-04 rates × the full block schedule with automation
   interruption mid-block.

6. **Resource exhaustion:** state chunk capped; editor scale clamped to
   [0.5, 3.0]; text-to-value entry points all clamp or fall back; the
   embedded table is fixed-size (64 512 bytes); no unbounded user-driven
   allocation exists in the product code.

7. **Overclaim hunting:** README (none exists — public description is an
   S-PACKAGE deliverable, recorded as a gap, not an overclaim), HANDOFF,
   GATE-G-001, editor UI strings, and VST3 metadata (`COMPANY_NAME
   "FaultCounter"`, categories `Fx Distortion`) contain no hardware-
   matched or WMD-affiliation claim. The product cannot overclaim
   fidelity in its current packaging because it ships no public
   packaging (G-002 blocked).

## Confirmed break + fix

| # | Sev | Break | Fix | Verification |
| --- | --- | --- | --- | --- |
| AR-01 | Medium | `setStateInformation()` called `pullSettings()` on the message thread, writing the plain-struct `settings_` concurrently with the audio thread's per-block read in `processBlock()` — a torn-read glitch vector on preset load during playback. The call was also redundant: `processBlock()` re-pulls every block. | Removed the call; state now becomes audible on the next audio callback via the audio-thread pull. | Full frozen suite re-run: 34/34, 22,507,698 assertions, rc=0 (count identical to baseline). |

## Accepted risks (probed, not broken)

- **AR-02** Low: `prepareToPlay` rebuilds the engine table copy; a host that
  calls it concurrently with `processBlock` violates the JUCE threading
  contract; standard practice, documented.
- **AR-03** Low: `lround(NaN)` on choice parameters (see probe 3) — no
  reachable path within host/APVTS contracts.
- **AR-04** Info: no new frozen tests were added for AR-01 because the
  existing StateTest/RobustnessTest/ReproducibilityTest dimensions already
  exercise state-restore-then-render; the fix removes a threading hazard
  rather than changing observable behavior, and the amendment procedure
  (§10) reserves new frozen tests for behavioral assertions.

## Verdict

One confirmed Medium break (AR-01), fixed and regression-verified against
the frozen suite. No Critical/High breaks remain. The product does not
overclaim hardware fidelity anywhere user-visible. **S-ADVERSARIAL
passes.**
