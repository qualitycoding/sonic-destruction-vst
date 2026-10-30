# RETROSPECTIVE.md — run-20261028T222647Z-geiger-vst3

- **Date (UTC):** 2026-10-30
- **Stage:** S-RETRO (plan/PLAN.md §S-RETRO). Record only — no deliverables
  are revised here.

## Outcome

The VST3 "Fault Counter" (Geiger Counter–style, software profile) reached a
complete, review-passed, evidence-backed state with public release held at
G-002. Fidelity is approximation-only (G-001 disposition). All frozen tests
green; both reviews pass; packaging artefacts prepared.

## Corrections and omissions, with L/K links

1. **Frozen tests contained defects, not just the implementation.**
   Six frozen-test defects (S-003…S-006 round), a seventh in T-018
   (hardcoded 9000 Hz contradicted the frozen T-001 mapping), and a reopen-
   contract gap were found and handled through the §10 challenge procedure
   with evidence, never by weakening tests.
   → `L-20261029T000000Z-frozen-tests-had-six-defects.md`,
   `L-20261030T075500Z-frozen-ui-tests-and-apvts-raw-store.md`,
   `TEST_CHALLENGE.md` rounds 3–4.
2. **Raw APVTS adapter stores bypass attachments.** Host-side and test-side
   `store()` on raw values never reaches parameter objects, so the editor
   must push raw values into attached controls explicitly on sync.
   → `L-20261030T075500Z-frozen-ui-tests-and-apvts-raw-store.md`.
3. **Sandbox recycling wiped `/tmp`/`/var/tmp` repeatedly (~16 recycles in
   one session)**, destroying builds, toolchain staging, and downloaded
   artefacts. Survival required: persistent ccache, checkpointed build
   loops on the persistent mount, exec-bit restoration after every copy,
   `-D` FetchContent overrides, and precompiling after the final configure.
   → `L-20261030T083000Z-sandbox-recycling-and-tmp-clearance.md`.
4. **A real DSP defect (SC-06) was found by fuzzing, not by inspection:**
   the processor bypass early-return let hostile NaN/Inf through untouched.
   Fixed by zeroing non-finite samples in the bypass path.
   → `FuzzTest.cpp` (frozen), `EXECUTION_LOG.md` S-009.
5. **Two latent defects survived to the review stages and were caught
   there:** `pullSettings()` let NaN `sampleRateHz` through `juce::jlimit`
   (R-01), and `setStateInformation()` raced `settings_` across threads
   (AR-01). Lesson: message-thread writes to plain structs read by the audio
   thread must be routed through the audio-thread pull; and `jlimit` is
   not a NaN guard.
   → `reviews/CODE_REVIEW.md` R-01, `reviews/ADVERSARIAL_REVIEW.md` AR-01.
6. **`operator new` replacement must live at global scope** to interpose;
   anonymous-namespace replacement fails at compile time.
   → `tests/integration/PerfTest.cpp` (frozen), S-009 log.
7. **Environment-blocking is not passing.** Local ASan+UBSan and real-DAW
   host coverage could not complete in the sandbox; both are documented as
   blocked and covered by the CI workflow instead of being silently
   treated as green.
   → `EXECUTION_LOG.md` S-009/S-014, `.github/workflows/validation.yml`.
8. **The first GitHub PAT died mid-run (API 401)** and its replacement
   needed the `workflow` scope to land the CI file; pushing immediately
   after each green commit is what kept the remote from drifting far.
   → `EXECUTION_LOG.md` push-status entries.
9. **Omission carried honestly:** R-08 (licence notices) was deferred to
   its owning stage and closed there; no root LICENCE existed until
   S-PACKAGE.
   → `packaging/NOTICES.md`, `LICENCE.md`.

## What went well

- The frozen-test-first protocol caught seven test-side defects before they
  could corrupt the implementation, and every amendment is evidenced.
- Checkpointed builds + persistent ccache let a ~35-minute-class JUCE build
  survive sandbox lifetimes measured in minutes.
- 100k-case fuzz, block-schedule reproducibility, and zero-allocation
  instrumentation gave high-confidence robustness evidence for a DSP
  product.

## Gate states at close

- G-001: `approximation-only` (hardware absent, record filed).
- G-002: `hold` (no proceed instruction; licence/packages pending).
- G-003: pending — S-KNOW follows this record.

## Schema check

Every finding above carries an L/K or record link; no deliverable was
modified in this stage. **S-RETRO complete.**
