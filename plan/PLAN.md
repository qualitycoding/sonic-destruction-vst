# Geiger Counter–Style VST3 Clone — Implementation Plan

**Status:** READY  
**Target repository:** `qualitycoding/sonic-destruction-vst`  
**Generation branch:** `gen-20261028T222647Z-geiger-vst3`  
**Active profile:** `software` (`software.deploys = false`; packaging is included, public release is gated)  
**Planning basis:** QualityCoding `planning-protocol` v3.4, consolidated into one handoff document because no target repository was supplied  
**Implementation rule:** Execute the steps below only; do not begin implementation during planning.  
**Working title:** **Fault Counter** (working name only; do not use WMD names, logos, panel artwork, or trade dress)

## 1. Goal and fixed scope

Build a cross-platform VST3 audio effect that recreates the signal flow and controls of the **original WMD Geiger Counter pedal**, not the Eurorack, Civilian Issue, or Pro variants. The official pedal description specifies a high-gain preamp, tone disable, sample-rate reduction, 1–8-bit depth, Bits/Mask operation, pre/post-table bit reduction, 252 hexadecimal-selected wave tables, CV control, and bypass.

### In scope

- Windows 10+ x64, macOS 11+ universal binary, and Linux x64 VST3.
- Mono processing; stereo operation as linked-parameter, dual-mono processing.
- Gain, Tone, Tone Enable, Sample Rate, Fine mode, Bit Depth/Mask, Bits/Mask mode, Pre/Post Table, 252 tables displayed `00`–`FB`, and Bypass.
- Host automation for every continuous or switched parameter; host automation replaces the pedal's CV jack.
- Clean-room procedural tables for development, plus a hardware-capture path for an accuracy-claimed version.
- Versioned preset/state restoration, deterministic offline rendering, real-time-safe processing, accessibility-aware UI, CI, pluginval, performance tests, and code/adversarial reviews.

### Out of scope

- VST2, AU, AAX, CLAP, standalone-only releases, iOS, and Android.
- Bass Mod, Eurorack CV jacks, Pro presets/MIDI, and Civilian Issue feature sets.
- Copying WMD firmware, branding, copyrighted panel art, or proprietary documentation into the product.
- Anti-aliased “improvements” that remove the pedal's intentional sample-rate foldover.
- Claims of sample-accurate or authentic matching without the hardware evidence defined below.

## 2. Measurable success criteria

- **SC-01:** Release VST3 builds succeed on all three desktop targets from a clean checkout.
- **SC-02:** Every listed pedal control is present, automatable, saved, restored, and reachable without a mouse-only interaction.
- **SC-03:** Tables `0…251` map one-to-one to display strings `00…FB`; each table contains exactly 256 unsigned 8-bit transfer values.
- **SC-04:** DSP implements the frozen conventions in §5 and passes all unit/integration tests at 44.1, 48, 88.2, 96, 176.4, and 192 kHz.
- **SC-05:** Bypass is sample-identical to input after the reported bypass latency, which must be zero.
- **SC-06:** No allocation, lock, file I/O, logging, or network access occurs in the audio callback; no NaN/Inf escapes for finite inputs in `[-100,100]` or for NaN/Inf hostile inputs.
- **SC-07:** Repeated offline renders with identical input, state, sample rate, and block schedule are bit-identical on the same build.
- **SC-08:** pluginval v1.0.4 exits `0` at strictness level 10 on each platform.
- **SC-09:** Release processing sustains at least 20× real time for one stereo instance at 48 kHz/64 frames on the pinned CI benchmark runner, with no underruns in the standalone stress harness.
- **SC-10:** State restoration passes randomized round trips and unknown future state chunks are rejected without memory unsafety or a crash.
- **SC-11:** A build labeled **Hardware-Matched** is permitted only when all 252×256 table codes are recovered and the replay residual is no more than 3 dB above the measured repeated-capture noise floor. Otherwise the product must be labeled **Geiger-Inspired Approximation**.
- **SC-12:** Critical/High security, correctness, real-time, code-review, and adversarial-review findings are closed before packaging.

## 3. Assumptions (`A-###`)

| ID | Fixed assumption | Consequence / audit |
|---|---|---|
| A-001 | Target is the original standard pedal. | Bass and other variants are excluded. Confirmed by the supplied product URL. |
| A-002 | VST3 is the only requested plug-in format. | No additional formats unless the goal is amended. |
| A-003 | JUCE 9.0.4 and C++20 are acceptable. | AGPL-3.0 source release is the default; a closed-source release requires an appropriate commercial JUCE licence before G-002. |
| A-004 | Hardware and a DC-coupled/low-distortion capture chain may be unavailable. | Development uses procedural tables; absence of capture evidence forces the approximation label. |
| A-005 | Host automation is an acceptable replacement for CV. | No virtual CV standard or MIDI mapping is invented. Hosts may link parameters to reproduce multi-target CV. |
| A-006 | Stereo dual-mono is a software convenience, not a hardware claim. | Each channel has independent state with linked parameters. |
| A-007 | Working sample-rate limits are 280 Hz and 58 kHz. | This range is not treated as hardware-verified; S-010 measures it and updates the mapping through the test-challenge procedure if wrong. |
| A-008 | Fine mode is a UI sensitivity mode, not a second DSP parameter. | Host automation always addresses actual Hz; Fine only changes knob response. |
| A-009 | A 5 Hz DC blocker is permissible output protection. | It is always active, documented, and tested; it must not be used to conceal unstable DSP. |
| A-010 | No prior QualityCoding knowledge-store location was provided. | R0 loads no prior items; S-KNOW proposes the target owner's `knowledge` repository. |

## 4. Decisions and public contracts (`D-###`)

### D-001 — Repository and toolchain

- CMake 3.30.5, Ninja 1.12.1, C++20.
- JUCE `9.0.4`, commit `3fb5cbc0340758596dac39f7c8de47d562b4896e`, fetched by commit rather than a floating branch.
- Catch2 `v3.16.1`, commit `08092139210881f01a34e60c5f2fd6afa0ea4024`.
- pluginval `v1.0.4`, commit `ed19c2c16b57a6d94db391bea3ef4a80b769d5bf`.
- GitHub Actions: `windows-2022`, `macos-14`, and `ubuntu-24.04`; Release and Debug CI jobs.
- Build commands:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DFAULTCOUNTER_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

If a package download fails, retry three times with exponential backoff; if the exact pinned commit cannot be fetched, halt with `BLOCKED.md`. Never silently move to another version.

### D-002 — Source layout

```text
CMakeLists.txt
cmake/Dependencies.cmake
src/PluginProcessor.{h,cpp}
src/PluginEditor.{h,cpp}
src/Parameters.{h,cpp}
src/dsp/Engine.{h,cpp}
src/dsp/Preamp.{h,cpp}
src/dsp/Tone.{h,cpp}
src/dsp/SampleHold.{h,cpp}
src/dsp/BitOps.{h,cpp}
src/dsp/Tables.{h,cpp}
src/dsp/DcBlocker.{h,cpp}
assets/tables/procedural.bin
assets/tables/hardware.bin              # required only for Hardware-Matched
tools/capture/README.md
tools/capture/extract_tables.py
tools/capture/fit_preamp.py
tests/unit/
tests/integration/
tests/golden/
tests/frozen/
tests/FROZEN_MANIFEST.sha256
```

### D-003 — Parameter schema

Use `juce::AudioProcessorValueTreeState`. IDs are permanent after the first release.

| ID | Type / range | Default | Mapping |
|---|---|---|---|
| `gain` | float `0…1` | `0.20` | `gainDb = 24v`; `drive = 8v²` |
| `tone` | float `0…1` | `0.50` | §D-006 |
| `toneEnabled` | bool | `true` | Bypass tone stage when false |
| `sampleRate` | float `280…58000` Hz | `58000` | `280 × (58000/280)^v` for normalized UI value `v` |
| `fine` | bool | `false` | UI-only: normalized knob `u` maps to full-range `v=0.5+0.5u`; automation always uses actual Hz |
| `bitControl` | float `0…1` | `1.0` | Bits: `1 + round(7v)`; Mask: `round(255v)` |
| `bitMode` | enum `Bits=0, Mask=1` | Bits | Selects interpretation of `bitControl` |
| `bitPosition` | enum `Post=0, Pre=1` | Post | Places bit operation before/after table |
| `table` | integer `0…251` | `0` | Display uppercase two-digit hex |
| `bypass` | bool | `false` | Zero-latency exact bypass |

Parameter text conversion must display Bits as `1…8 bit`, Mask as `000…255`, Sample Rate in Hz/kHz, and Table as `00…FB`. Reject non-finite incoming normalized values by clamping to the default; clamp all finite values to range. Switching Fine preserves the audible Hz value. If the current full-range value is `v≥0.5`, initialize the Fine knob to `u=2v-1`; otherwise initialize it to `u=0` and leave the value unchanged until the user moves the knob. Leaving Fine restores the coarse knob from the unchanged actual value.

### D-004 — Numeric conventions and 8-bit operations

All public DSP uses normalized float samples. Let:

```text
sat01(x) = min(1, max(0, x))
toCode(x) = round(255 × sat01((x + 1) / 2))             # uint8, 0…255
fromCode(c) = 2 × (c / 255) - 1                         # float, -1…+1
```

For Bits mode with `b = 1…8`:

```text
levels = 2^b - 1
q = round((c / 255) × levels)
Bits(c,b) = round((q / levels) × 255)
```

This gives identity at 8 bits and codes `{0,255}` at 1 bit. For Mask mode with `m = 0…255`, `Mask(c,m) = c bitwise-and m`. These definitions are deterministic, integer-based, and shared by pre/post operations. The hardware capture must challenge these conventions rather than being forced to fit them.

### D-005 — Sample-rate reduction

Use a continuous-phase zero-order hold per channel:

```text
effectiveRate = min(max(sampleRateHz, 1), hostSampleRate)
phase += effectiveRate / hostSampleRate
if phase >= 1:
    held = currentInput
    phase -= floor(phase)
output = held
```

Initialize `phase = 1` and `held = 0` on `prepareToPlay`, so the first input sample is captured. Do not interpolate and do not insert an anti-alias filter. Parameter automation changes the increment, not the phase. If `sampleRateHz >= hostSampleRate`, the stage is exactly transparent. Reset all channel phases together.

### D-006 — Fallback preamp and tone model

This is an explicitly replaceable approximation until S-012:

1. Input gain `g = 10^(gainDb/20)`.
2. If Tone Enable is on:
   - `tone < 0.5`: crossfade low-pass output (`fc = 900 Hz`) with dry using `2 × tone`;
   - `tone = 0.5`: dry;
   - `tone > 0.5`: crossfade dry with high-pass output (`fc = 1200 Hz`) using `2 × (tone - 0.5)`.
   Use JUCE second-order Butterworth coefficients, Q = `1/sqrt(2)`, recomputed outside the inner sample loop.
3. Soft clip: if `drive < 10^-6`, `y = g × filtered`; otherwise `y = tanh(drive × g × filtered) / tanh(drive)`.
4. Tone disabled bypasses only the filters, not gain/saturation.

S-012 may replace coefficient values or the complete model only through the frozen-test amendment procedure. No undocumented “magic” waveshaping is permitted.

### D-007 — Engine order and output protection

For each channel:

```text
input sanitization
→ fallback/calibrated preamp and tone
→ zero-order hold
→ 8-bit code conversion
→ bit operation if Pre
→ selected 256-entry transfer table
→ bit operation if Post
→ normalized float conversion
→ 5 Hz DC blocker
→ finite-value guard
```

Stereo channels share the selected table and parameters but not filter, hold, or DC-blocker state. Bypass returns the original input before any processing. No output limiter, mix control, oversampling, or hidden noise gate.

### D-008 — Tables and procedural fallback

`assets/tables/procedural.bin` is exactly 64,512 bytes (`252 × 256`), row-major by table index. The generator uses only explicit-width integer arithmetic. For variant `k=0…27`, let `p=k+1`, input code be `c`, and centered input be `x=c-128`. `clampS(v)=clamp(v,-128,127)` and output code is `clampS(y)+128`; `mod` is floor-modulus. The nine families, each with 28 variants, are:

1. **Saturation:** `y=clampS(x×p)`.
2. **Signed power:** `e=1+(k mod 4)`, `a=1+floor(k/4)`; `y=sign(x)×min(127, round((a×abs(x)^e)/128^(e-1)))`, using 128-bit intermediates.
3. **Chebyshev:** `n=1+(k mod 7)`, `a=1+floor(k/7)`; compute `T_n` in Q15 with `T0=32768`, `T1=x×256`, `Tn=((2×T1×T(n-1))>>15)-T(n-2)`; `y=clampS((a×Tn)>>8)`.
4. **Triangle fold:** `z=mod(x×p+128,512)`; `t=z` when `z<256`, otherwise `511-z`; `y=t-128`.
5. **Modulo wrap:** `y=mod(x×p+128,256)-128`.
6. **XOR:** output code `c XOR ((73×p) mod 256)`.
7. **Rotation/reversal:** `r=1+(k mod 7)`, group `g=floor(k/7)`; form a base code by rotate-left for `g=0`, rotate-right for `g=1`, bit-reverse then rotate-left for `g=2`, or Gray code `c XOR (c>>1)` then rotate-left for `g=3`, always by `r` bits; output the base code `XOR (17k mod 256)`. The XOR term guarantees 28 unique variants without changing the intended bit-rotation family.
8. **Signed multiplication:** `y=sign(x)×min(127, (x²×p)>>14)`.
9. **Asymmetric threshold:** `t=8+4×(k mod 7)`, `a=1+floor(k/7)`; for `x≥0`, `y=min(127,4×a×max(0,x-t))`; otherwise `y=max(-128,-2×a×max(0,-x-2t))`.

Within each family, sort rows by `severity=sum_c(abs(y_c-x_c))+256×N`, where `N` counts sign changes between successive first differences `y_(c+1)-y_c`; ties retain lower `k`. Concatenate families in the order above. The generator writes formula IDs, `k`, severity, and row hashes to its manifest. S-005 freezes the resulting asset hash. Procedural tables are a scaffold, never represented as WMD's actual tables.

For a Hardware-Matched build, `hardware.bin` replaces the procedural resource and carries a SHA-256 manifest linking every row to its capture files. Missing or duplicate table rows block S-013.

### D-009 — Real-time and state rules

- Precompute tables, filters, and parameter ramps in `prepareToPlay` or on the message thread; audio-codec work is forbidden in the callback.
- Use atomics or JUCE parameter attachments correctly; no locks in `processBlock`.
- Smooth Gain and Tone over 5 ms; Sample Rate over 1 ms. Table, Bits/Mask, Pre/Post, and Tone Enable switch on block boundaries in v1.
- Use `juce::ScopedNoDenormals`; replace non-finite input and output with `0`.
- State chunk: magic `FCNT`, schema version `1`, payload length, then APVTS XML. Enforce a 1 MiB maximum, validate all values, and ignore unrecognized XML elements for forward tolerance. Never execute or deserialize object graphs.
- Mutation testing is opted out (`D-010`) because the C++ DSP suite combines golden vectors, sanitizer fuzzing, pluginval, and hardware coverage; revisit if those gates prove insufficient.

## 5. Frozen acceptance tests (`T-###`)

The implementation agent must materialize these tests and fixtures without weakening them, add `FROZEN — DO NOT MODIFY` headers, run them red against stubs, then freeze SHA-256 hashes in `tests/FROZEN_MANIFEST.sha256`. Any challenge follows §10.

| ID | Category | Frozen assertion |
|---|---|---|
| T-001 | Unit | Parameter IDs, ranges, defaults, text formatting, and `00…FB` table mapping match D-003. |
| T-002 | Unit | `toCode/fromCode` covers 0, 127, 128, 255 and out-of-range inputs; all outputs remain finite and bounded. |
| T-003 | Unit | Bits is identity at 8, two-level at 1, and exact for golden codes at 2–7; `Mask(0xAC,0xF0)==0xA0`. |
| T-004 | Unit | SampleHold captures the first sample, is transparent at/above host rate, and produces analytically scheduled holds at 280 Hz and 1 kHz. |
| T-005 | Unit | Each table has 256 entries; index count is 252; procedural asset length and frozen SHA-256 match. |
| T-006 | Unit | Pre and Post routes match hand-computed golden vectors using a reversible test table and a non-reversible test table. |
| T-007 | Unit | Gain 0 with Tone disabled is linear; maximum gain remains bounded; Tone midpoint approximates dry within filter coefficient tolerance `1e-6`. |
| T-008 | Unit | DC blocker rejects a constant input to below `-80 dBFS` after 10 seconds at 48 kHz and remains stable at all test rates. |
| T-009 | Integration | The complete chain matches committed golden PCM fixtures for impulse, ramp, sine, noise seed `0x47C0`, and silence. |
| T-010 | Integration | Bypass is bit-identical for mono/stereo and all block sizes. |
| T-011 | Integration | State round trips preserve every parameter and table; malformed/truncated/oversized chunks fail safely. |
| T-012 | Integration | Mono and stereo buses negotiate correctly; stereo channels do not leak state in an impulse-on-left-only test. |
| T-013 | Operational | Rates in SC-04, blocks `{1,2,7,16,64,511,4096,8192}`, repeated prepare/reset, and interrupted automation produce no crash/NaN. |
| T-014 | Security | 100,000 seeded malformed state/parameter fuzz cases under ASan+UBSan produce no finding; dependency scan has no unaccepted Critical/High advisory. |
| T-015 | Performance | Allocation/lock instrumentation reports zero audio-callback events; SC-09 benchmark passes on three repeated runs with ≤10% relative variance. |
| T-016 | Reproducibility | Two same-build offline renders are bit-identical; changing block partition does not alter output except during the explicitly documented parameter-smoothing transition. |
| T-017 | Host | `pluginval --strictness-level 10 <path-to-vst3>` exits `0`. |
| T-018 | UI | At 100%, 150%, and 200% UI scale, every control is visible, labeled, keyboard reachable, and matches APVTS after edit/reopen. |
| T-019 | Hardware | Capture chain noise floor is at or below `-50 dBFS`; repeated captures establish a per-fixture noise floor before fitting. |
| T-020 | Hardware | All `252×256` hardware table codes have exactly one evidenced value; unresolved codes fail the test rather than being interpolated silently. |
| T-021 | Hardware | Hardware replay residual is ≤ repeated-capture noise floor + 3 dB for every fixture; any failed fixture blocks Hardware-Matched status. |
| T-022 | Packaging | Clean-machine VST3 scan/load/state/render smoke test passes in REAPER on all three OS targets; one additional VST3 host per OS is recorded by version. |
| T-023 | Regression | Golden PCM and report files embed commit, environment-lock hash, parameter state, sample rate, block schedule, seed, and UTC generation time. |

Numeric tolerances for floating-point filter comparisons are `max(1e-6, 32ε)` for float and `max(1e-12, 32ε)` for double reference calculations; integer code and table comparisons are exact. Hardware tolerances derive only from repeated-capture noise, not an arbitrary fidelity threshold.

## 6. Execution steps

### S-000 Environment & access verification
- **Tier:** Haiku
- **Profile:** software
- **Depends on:** none
- **Inputs:** this plan; target repository URL and credentials supplied outside the product source.
- **Actions:**
  1. Create `gen-<UTC>-geiger-vst3`, commit this plan and its separated appendices if a repository is used, then create `impl-<run_id>`.
  2. Verify Git push permission, GitHub Actions, release-upload permission, network access to GitHub, and the three build environments.
  3. Install and print exact tool versions; verify JUCE/Catch2/pluginval commit hashes.
  4. Create `EXECUTION_LOG.md` and `.checkpoints/state.json`; checkpoint after every step and before long capture jobs.
- **Outputs:** implementation branch, verified environment report, execution log, initial checkpoint.
- **Evidence produced:** none.
- **Done when:** setup commands succeed and every hash/version matches D-001.
- **Checkpoint:** branch, tool versions, permissions, artifact hashes, next step `S-001`.
- **On failure:** halt before delivery work and write `BLOCKED.md`; do not substitute tools.
- **Gate:** none.
- **Relevant decisions/claims:** D-001; C-003/C-004.
- **Lessons applied:** none loaded.
- **Exclusive resources:** target Git repository.

### S-001 Materialize and red-verify frozen tests
- **Tier:** Sonnet
- **Profile:** software
- **Depends on:** S-000
- **Inputs:** §5; D-002/D-003/D-004/D-005.
- **Actions:** create public stubs, deterministic fixtures, and T-001…T-023 harnesses; run all applicable non-hardware tests; confirm each fails by assertion/not-implemented rather than syntax or missing-fixture errors; hash and freeze the suite.
- **Outputs:** test tree, stubs, red-run log, `tests/FROZEN_MANIFEST.sha256`.
- **Evidence produced:** all non-hardware T IDs in red state.
- **Done when:** `sha256sum -c tests/FROZEN_MANIFEST.sha256` succeeds and red verification is valid.
- **Checkpoint:** freeze hash and test counts.
- **On failure:** repair test infrastructure only; never delete a failing requirement.
- **Gate:** none.
- **Relevant decisions/claims:** D-002…D-009.
- **Lessons applied:** none.
- **Exclusive resources:** test manifest.

### S-002 Scaffold JUCE plug-in and parameter/state layer
- **Tier:** Sonnet
- **Profile:** software
- **Depends on:** S-001
- **Inputs:** D-001/D-002/D-003/D-009.
- **Actions:** implement CMake targets, VST3 metadata (`Fx Distortion`), buses, APVTS schema, parameter text functions, versioned state parser, and a transparent processor.
- **Outputs:** buildable VST3, parameter/state sources, initial CI build.
- **Evidence produced:** T-001, T-010, T-011, T-012.
- **Done when:** those tests pass; transparent audio remains bit-identical.
- **Checkpoint:** build commit and passing IDs.
- **On failure:** roll back to last green commit; fix the interface, not the test.
- **Gate:** none.
- **Relevant decisions/claims:** D-001/D-003/D-009; C-003.
- **Lessons applied:** none.
- **Exclusive resources:** none.

### S-003 Implement and verify zero-order hold
- **Tier:** Sonnet
- **Profile:** software
- **Depends on:** S-001
- **Inputs:** D-005.
- **Actions:** implement `SampleHold`, prepare/reset behavior, host-rate clamp, automation phase continuity, and analytic timing tests.
- **Outputs:** `src/dsp/SampleHold.*` and golden timing vectors.
- **Evidence produced:** T-004, T-013 (partial).
- **Done when:** all SampleHold vectors pass at every SC-04 host rate.
- **Checkpoint:** DSP API hash and passing IDs.
- **On failure:** use a double-precision phase reference; if mismatch persists, halt with `TEST_CHALLENGE.md` rather than retiming fixtures.
- **Gate:** none.
- **Relevant decisions/claims:** D-005.
- **Lessons applied:** none.
- **Exclusive resources:** none.

### S-004 Implement bit depth, mask, and routing primitives
- **Tier:** Sonnet
- **Profile:** software
- **Depends on:** S-001
- **Inputs:** D-004/D-007.
- **Actions:** implement integer code conversion, Bits/Mask operations, pre/post dispatch, and hostile-input guards.
- **Outputs:** `src/dsp/BitOps.*`, routing golden vectors.
- **Evidence produced:** T-002, T-003, T-006.
- **Done when:** exact integer assertions pass and routing order is proven with both test tables.
- **Checkpoint:** passing IDs and source hashes.
- **On failure:** stop on any signed-overflow/undefined-behavior finding; use explicit unsigned arithmetic.
- **Gate:** none.
- **Relevant decisions/claims:** D-004/D-007.
- **Lessons applied:** none.
- **Exclusive resources:** none.

### S-005 Freeze procedural table generator and asset
- **Tier:** Opus
- **Profile:** software
- **Depends on:** S-001
- **Inputs:** D-008.
- **Actions:** transcribe the nine-family integer generator in D-008 into `assets/tables/GENERATOR.md`; implement it literally; verify row count, row uniqueness, determinism, severity ordering, and asset hash; commit source and binary.
- **Outputs:** procedural table asset, generator, manifest, hash.
- **Evidence produced:** T-005.
- **Done when:** two clean generations are byte-identical and the asset is 64,512 bytes.
- **Checkpoint:** table SHA-256.
- **On failure:** if a family cannot produce 28 distinct rows, halt and amend D-008; do not pad with duplicates.
- **Gate:** none.
- **Relevant decisions/claims:** D-008.
- **Lessons applied:** none.
- **Exclusive resources:** procedural asset.

### S-006 Implement fallback preamp, tone, and DC protection
- **Tier:** Opus
- **Profile:** software
- **Depends on:** S-001
- **Inputs:** D-006/D-007.
- **Actions:** implement filters, gain/drive laws, denormal handling, 5 Hz DC blocker, coefficient update policy, and analytic/golden tests.
- **Outputs:** `Preamp.*`, `Tone.*`, `DcBlocker.*`, double-precision reference vectors.
- **Evidence produced:** T-007, T-008.
- **Done when:** all numerical tests pass across SC-04 rates.
- **Checkpoint:** model version and golden hashes.
- **On failure:** if filter stability fails at any rate, switch to a documented topology-preserving transform implementation and regenerate references through test amendment.
- **Gate:** none.
- **Relevant decisions/claims:** D-006/D-007.
- **Lessons applied:** none.
- **Exclusive resources:** none.

### S-007 Integrate the complete engine
- **Tier:** Opus
- **Profile:** software
- **Depends on:** S-002, S-003, S-004, S-005, S-006
- **Inputs:** D-007/D-009 and all DSP modules.
- **Actions:** connect processor and engine; add parameter smoothing, dual-mono state, bypass, sanitization, golden render CLI, and real-time instrumentation.
- **Outputs:** complete functional plug-in and offline renderer.
- **Evidence produced:** T-009…T-016.
- **Done when:** all applicable tests pass, including zero callback allocation/lock events and deterministic renders.
- **Checkpoint:** engine commit and golden manifest.
- **On failure:** bisect by stage using golden vectors; never tune the expected PCM to the implementation.
- **Gate:** none.
- **Relevant decisions/claims:** D-004…D-009.
- **Lessons applied:** none.
- **Exclusive resources:** golden fixture directory.

### S-008 Build accessible, brand-independent UI
- **Tier:** Sonnet
- **Profile:** software
- **Depends on:** S-002, S-005
- **Inputs:** D-003 and control inventory.
- **Actions:** create original pedal-inspired layout; implement rotary controls, hex display, mode toggles, Fine behavior, keyboard/focus support, value entry, tooltips, 100/150/200% scaling, and host synchronization.
- **Outputs:** editor sources and UI test captures.
- **Evidence produced:** T-002 (display portion), T-018.
- **Done when:** all controls remain usable and APVTS synchronized at every scale.
- **Checkpoint:** UI commit and screenshot hashes.
- **On failure:** use a generic accessible JUCE control rather than blocking on custom graphics.
- **Gate:** none.
- **Relevant decisions/claims:** D-003; C-001/C-002.
- **Lessons applied:** none.
- **Exclusive resources:** UI golden images.

### S-009 Add CI, sanitizer, vulnerability, and performance jobs
- **Tier:** Sonnet
- **Profile:** software
- **Depends on:** S-007
- **Inputs:** D-001, SC-08/SC-09/SC-12.
- **Actions:** add three-OS builds, ASan/UBSan Linux job, deterministic fuzz job, dependency advisory scan, benchmark, artifact upload, and pluginval level-10 jobs.
- **Outputs:** CI workflows and downloadable validation reports.
- **Evidence produced:** T-013…T-017, T-023.
- **Done when:** all required jobs are green on the same commit.
- **Checkpoint:** workflow run URLs and result hashes.
- **On failure:** a Critical/High advisory or sanitizer finding halts downstream work; performance misses use D-011 below.
- **Gate:** none.
- **Relevant decisions/claims:** D-001/D-009/D-010.
- **Lessons applied:** none.
- **Exclusive resources:** CI runner and benchmark lock.

### S-010 Characterize hardware, or record approximation-only evidence
- **Tier:** Opus
- **Profile:** software
- **Depends on:** S-007
- **Inputs:** original pedal; audio interface; isolated re-amp/DI chain as applicable; fixed capture scripts.
- **Actions:**
  1. Measure chain noise and repeatability before connecting the pedal; require T-019.
  2. With Tone disabled, post 8-bit depth, maximum sample rate, and bypassed table candidates identified by capture, measure actual sample-rate and knob endpoints using recorded test signals.
  3. Capture slow bipolar ramps and code-resolution stimuli for every table; store raw immutable WAVs and settings manifests.
  4. If hardware is absent, skip capture without fabricating data and write `research/HARDWARE_UNAVAILABLE.md`.
- **Outputs:** raw captures, environment/settings manifests, noise-floor report, or unavailability record.
- **Evidence produced:** T-019; input to T-020/T-021.
- **Done when:** every capture has a complete manifest and checksum, or approximation-only status is recorded.
- **Checkpoint:** after every 16 tables; raw capture hashes and resume index.
- **On failure:** if chain noise exceeds `-50 dBFS`, fix gain staging/clocking and repeat; never denoise raw evidence.
- **Gate:** none.
- **Relevant decisions/claims:** A-004/A-007; C-005/C-006/C-007.
- **Lessons applied:** none.
- **Exclusive resources:** the single hardware pedal and capture interface.

### S-011 Extract hardware tables without inventing missing codes
- **Tier:** Opus
- **Profile:** software
- **Depends on:** S-010
- **Inputs:** immutable ramp captures and calibration manifests.
- **Actions:** detect ADC plateaus, map input codes to output codes, retain ambiguity flags, require one evidenced output per table row, generate `hardware.bin` plus provenance JSON, and independently replay a random 10% sample of tables.
- **Outputs:** hardware table asset, extraction report, unresolved-code report.
- **Evidence produced:** T-020.
- **Done when:** all 64,512 cells have evidence and replay agrees exactly at the code level.
- **Checkpoint:** after each extraction batch; table row hashes.
- **On failure:** unresolved cells block Hardware-Matched status; additional targeted gain/offset captures are allowed, interpolation is not.
- **Gate:** none.
- **Relevant decisions/claims:** D-008; C-007.
- **Lessons applied:** none.
- **Exclusive resources:** capture data and hardware asset.

### S-012 Calibrate preamp/tone and compare complete signal chain
- **Tier:** Opus
- **Profile:** software
- **Depends on:** S-010, S-011
- **Inputs:** sweeps, sine/noise/impulse fixtures, repeated-capture noise estimates.
- **Actions:** fit the fewest model parameters that meet the noise-derived threshold on training fixtures; validate on held-out fixtures; freeze model and residuals; do not iterate against validation data.
- **Outputs:** calibrated model constants, training/validation split, residual report, golden hardware comparisons.
- **Evidence produced:** T-021.
- **Done when:** every held-out fixture meets noise floor +3 dB, or the failure is carried unchanged to G-001.
- **Checkpoint:** fitted parameter hash and validation report.
- **On failure:** do not widen tolerance; choose approximation-only or stop at the gate.
- **Gate:** none.
- **Relevant decisions/claims:** D-006; C-006/C-007.
- **Lessons applied:** none.
- **Exclusive resources:** hardware fixtures.

### S-013 Fidelity disposition gate
- **Tier:** Opus
- **Profile:** software
- **Depends on:** S-011, S-012 if hardware exists; otherwise S-010
- **Inputs:** T-019…T-021 reports or hardware-unavailability record.
- **Actions:** write `GATE-G-001.md` with table coverage, residuals, failed fixtures, proposed product label, and licensing statement.
- **Outputs:** signed gate record and selected fidelity track.
- **Evidence produced:** upgrades SC-11 disposition.
- **Done when:** human response is `hardware-matched`, `approximation-only`, or `stop`.
- **Checkpoint:** gate response and downstream branch.
- **On failure:** no response means approximation-only development may continue, but public release remains blocked.
- **Gate:** G-001.
- **Relevant decisions/claims:** SC-11; A-004.
- **Lessons applied:** none.
- **Exclusive resources:** none.

### S-014 DAW acceptance and package rehearsal
- **Tier:** Sonnet
- **Profile:** software
- **Depends on:** S-008, S-009, S-013
- **Inputs:** release candidate VST3, T-022 host matrix.
- **Actions:** test clean-machine scan/load/bypass/automation/state/render; rehearse unsigned local package installation; record host and OS versions; verify no WMD trademark/artwork is bundled.
- **Outputs:** host acceptance report and rehearsed packages.
- **Evidence produced:** T-022.
- **Done when:** every required smoke test passes and package contents match the SBOM.
- **Checkpoint:** package hashes and host matrix.
- **On failure:** packaging errors return to S-009; host-specific DSP errors return to S-007.
- **Gate:** none.
- **Relevant decisions/claims:** SC-01/SC-02/SC-12.
- **Lessons applied:** none.
- **Exclusive resources:** clean test machines.

### S-REVIEW Traditional code review
- **Tier:** Opus, fresh context
- **Profile:** software
- **Depends on:** S-014
- **Inputs:** full implementation diff, frozen tests, CI reports, dependency releases/advisories.
- **Actions:** review correctness, real-time safety, undefined behavior, duplication, state security, coverage gaps, licence notices, and dependency versions; link every finding to a fix or accepted risk.
- **Outputs:** `reviews/CODE_REVIEW.md` and fix commits.
- **Evidence produced:** review record required by SC-12.
- **Done when:** no unresolved Critical/High finding and every new code path maps to a frozen test.
- **Checkpoint:** review hash and residual findings.
- **On failure:** return to the owning step; new tests require the amendment process.
- **Gate:** none.
- **Relevant decisions/claims:** all.
- **Lessons applied:** all recorded during execution.
- **Exclusive resources:** none.

### S-ADVERSARIAL Adversarial review
- **Tier:** Fable (Mythos) or Opus, fresh context
- **Profile:** software
- **Depends on:** S-REVIEW
- **Inputs:** release candidate, all assumptions, hostile state/audio/automation cases.
- **Actions:** challenge A-001…A-010, fuzz malformed state and parameter sequences, force resource exhaustion, probe denormals/NaN/Inf, vary block schedules, and attempt to make the product overclaim hardware fidelity.
- **Outputs:** `reviews/ADVERSARIAL_REVIEW.md`, reproduced breaks, new tests or accepted risks.
- **Evidence produced:** adversarial record required by SC-12.
- **Done when:** all confirmed Critical/High breaks are fixed and frozen regression tests pass.
- **Checkpoint:** break count and disposition.
- **On failure:** halt packaging for Critical/High issues.
- **Gate:** none.
- **Relevant decisions/claims:** all A/D/C items.
- **Lessons applied:** all recorded during execution.
- **Exclusive resources:** none.

### S-PACKAGE Build reproducible release artifacts
- **Tier:** Sonnet
- **Profile:** software
- **Depends on:** S-ADVERSARIAL
- **Inputs:** reviewed commit and selected fidelity label.
- **Actions:** build clean VST3 bundles; generate SBOM, licence notices, source archive where required, SHA-256 sums, and release notes stating either Hardware-Matched evidence or approximation limitations.
- **Outputs:** three platform packages, SBOM, notices, checksums, draft release notes.
- **Evidence produced:** final SC-01/SC-11/SC-12 bundle.
- **Done when:** package hashes reproduce from the same commit and environment; package scans/loads once more.
- **Checkpoint:** artifact hashes.
- **On failure:** destroy inconsistent artifacts and rebuild; do not hand-edit binaries or reports.
- **Gate:** none.
- **Relevant decisions/claims:** A-003/A-004; SC-11.
- **Lessons applied:** all.
- **Exclusive resources:** release artifact directory.

### S-RELEASE Optional public release
- **Tier:** Haiku
- **Profile:** software
- **Depends on:** S-PACKAGE
- **Inputs:** complete release evidence bundle.
- **Actions:** write `GATE-G-002.md`; release only after explicit `proceed` with a named version and target.
- **Outputs:** public release only if approved; otherwise no external action.
- **Evidence produced:** release record or hold record.
- **Done when:** approved upload succeeds and hashes match, or `stop` is recorded.
- **Checkpoint:** release URL/version or hold status.
- **On failure:** no retry against a different channel; preserve artifacts and report the failed action.
- **Gate:** G-002.
- **Relevant decisions/claims:** all release criteria.
- **Lessons applied:** all.
- **Exclusive resources:** release channel.

### S-RETRO Retrospective
- **Tier:** Opus, fresh context
- **Profile:** software
- **Depends on:** every preceding step including S-RELEASE when approved
- **Inputs:** execution log, commits, deviations, challenges, gates, CI, reviews, lessons, and knowledge.
- **Actions:** identify corrections and omissions without rerunning or changing deliverables; link every finding to a lesson or knowledge item.
- **Outputs:** `RETROSPECTIVE.md` and missing L/K records.
- **Evidence produced:** none.
- **Done when:** every finding has an L/K link and schema checks pass.
- **Checkpoint:** retrospective completion.
- **On failure:** complete the record before S-KNOW; do not revise released artifacts here.
- **Gate:** none.
- **Relevant decisions/claims:** all.
- **Lessons applied:** all.
- **Exclusive resources:** none.

### S-KNOW Lessons and knowledge push
- **Tier:** Haiku
- **Profile:** software
- **Depends on:** S-RETRO
- **Inputs:** `lessons/`, `knowledge/`, proposed owner `knowledge` repository.
- **Actions:** write `GATE-G-003.md`; on approval push separate-item files and regenerate indexes; on rejection rebase and retry up to three times; otherwise create a Git bundle fallback.
- **Outputs:** pushed knowledge or `knowledge-<run_id>.bundle`.
- **Evidence produced:** none.
- **Done when:** target indexes list every ID, `do-not-push` is recorded, or fallback bundle exists.
- **Checkpoint:** final state; execution halted.
- **On failure:** use bundle fallback and report its path.
- **Gate:** G-003.
- **Relevant decisions/claims:** A-010.
- **Lessons applied:** all.
- **Exclusive resources:** knowledge repository.

## 7. Additional decision rules

- **D-011 performance miss:** profile first; optimize table lookup, filter coefficient updates, and per-sample work in that order. If still below 20× real time, reduce UI/timer work and re-benchmark. Never remove intentional aliasing, safety checks, or tests. Persistent failure blocks release.
- **D-012 unexpected host behavior:** reproduce in the offline harness and pluginval, add a frozen regression test, and choose the smallest VST3-compliant fix. Host-specific bypasses require a documented version check and Opus review.
- **D-013 capture contradiction:** preserve raw evidence and report the contradiction. If tables behave non-statically, split the model into static transfer plus measured stateful stage through plan amendment; do not average behaviors into a LUT.
- **D-014 hardware unavailable:** continue procedural development and label all output as approximation; do not claim recovered range, tone, or tables.
- **D-015 security finding:** Critical/High blocks packaging. Medium/Low requires a written risk acceptance with owner and review date.
- **D-016 unanticipated situation:** choose the most reversible, scope-neutral option and log `DEVIATIONS.md`; halt with `BLOCKED.md` if it touches frozen tests, security, data integrity, public interfaces, provenance, or fidelity claims.

## 8. Human gates

| Gate | Trigger | Evidence bundle | Allowed responses and branches |
|---|---|---|---|
| G-001 | S-013 | Table coverage, raw-data hashes, capture noise, held-out residuals, unavailable record if applicable | `hardware-matched`: use hardware asset and label; `approximation-only`: retain procedural asset and explicit label; `stop`: end delivery |
| G-002 | S-RELEASE | Packages, hashes, SBOM, licences, green CI, host matrix, both reviews, fidelity label | `proceed: <version> <target>`: upload once; `stop`: retain private artifacts |
| G-003 | S-KNOW | Lesson/knowledge IDs, tags, secret scan, proposed store | `push-to-proposed`; `push-to: <target>`; `do-not-push` |

While waiting at a gate, independent work may continue only when it does not depend on the gate outcome. No external or irreversible action is allowed while G-002 is unresolved.

## 9. Traceability

| Requirement | Tests | Implementing/verifying steps |
|---|---|---|
| SC-01 | T-017, T-022 | S-000, S-002, S-009, S-014, S-PACKAGE |
| SC-02 | T-001, T-011, T-018 | S-002, S-008 |
| SC-03 | T-001, T-005, T-020 | S-005, S-011 |
| SC-04 | T-002…T-009, T-013, T-016 | S-003…S-007 |
| SC-05 | T-010 | S-002, S-007 |
| SC-06 | T-013, T-014, T-015 | S-004, S-006, S-007, S-009 |
| SC-07 | T-016, T-023 | S-007, S-009 |
| SC-08 | T-017 | S-009 |
| SC-09 | T-015 | S-007, S-009 |
| SC-10 | T-011, T-014 | S-002, S-009 |
| SC-11 | T-019…T-021 | S-010…S-013 |
| SC-12 | T-014…T-017, review records | S-009, S-REVIEW, S-ADVERSARIAL |

## 10. Freeze, deviation, and resumption protocol

1. After red verification, add `FROZEN — DO NOT MODIFY` to every test/fixture and write SHA-256 hashes.
2. An implementing agent may not edit, skip, expected-fail, or weaken a frozen test.
3. A suspected invalid test requires `TEST_CHALLENGE.md` containing evidence and a proposed fix. Re-research the underlying claim, amend dependent decisions/steps, red-verify, re-freeze, cold-read the changed plan, and run one new pre-mortem round.
4. Record corrections as timestamped lesson files with trigger, mistake, correction, prevention rule, detection check, evidence, and tags (`domain:audio`, `tech:cpp`, `tech:juce`, `domain:dsp`).
5. Record newly discovered technical facts as knowledge items with source, locator, applicable version, confidence, and originating claim. Supersede wrong facts; never erase their history.
6. Every step updates `.checkpoints/state.json` atomically with run ID, branch, sequence, active step, completed/pending work, artifact hashes, open items, capture resume index, and UTC timestamp.
7. On resume, verify every artifact hash, reconcile any interrupted external action, and continue idempotently from the active step. Raw captures and generated reports use temporary-file-then-rename writes.

## 11. Research register

### Claims

| ID | Claim | Confidence / use |
|---|---|---|
| C-001 | Official page describes Gain, Tone/Disable, Sample Rate, Bit Depth, Bits/Mask, Wave Table, CV, and Bypass. | Verified against official product page; informs control inventory. |
| C-002 | Official page specifies 252 wave tables and hexadecimal display. | Verified; informs D-003/D-008. |
| C-003 | JUCE supports VST3 and CMake; pluginval supports headless VST3 validation at strictness 1–10. | Verified against official repositories; informs D-001/T-017. |
| C-004 | JUCE is available under AGPLv3 and commercial licensing. | Verified against official repository; release licensing remains gated. |
| C-005 | Exact original-pedal sample-rate endpoints and Fine mapping. | Not fully verified; A-007/A-008 are working conventions challenged in S-010. |
| C-006 | Exact analog preamp/tone behavior and internal stage ordering beyond official descriptions. | Unverifiable from marketing material; measured in S-010/S-012. |
| C-007 | Every wave table is a static 256-code function with no hidden state. | Unverifiable until capture; contradiction invokes D-013. |

### Tier-1 sources

- WMD, official Geiger Counter product/control documentation: https://wmdevices.com/products/geiger-counter
- QualityCoding, planning protocol: https://github.com/qualitycoding/skills/blob/main/protocols/planning-protocol.md
- JUCE 9.0.4 source and licence: https://github.com/juce-framework/JUCE/tree/9.0.4
- JUCE CMake audio plug-in example: https://github.com/juce-framework/JUCE/tree/9.0.4/examples/CMake/AudioPlugin
- pluginval v1.0.4 source and CLI documentation: https://github.com/Tracktion/pluginval/tree/v1.0.4
- Catch2 v3.16.1 source: https://github.com/catchorg/Catch2/tree/v3.16.1

Research saturation was reached for architecture and control inventory. Hardware-only claims intentionally remain unverified and are gated rather than presented as facts.

## 12. Recursive pre-mortem and assumption audit

| Risk | Severity | Failure imagined despite perfect execution | Mitigation / residual |
|---|---|---|---|
| R-001 missing tables | High | Plug-in ships with 252 names but inaccurate sonic identity. | Full-cell evidence, exact replay, G-001; unresolved cells block Hardware-Matched. |
| R-002 analog mismatch | High | Correct tables still sound unlike pedal because preamp/tone dominates. | Train/held-out capture split, noise-derived tolerance, approximation fallback. |
| R-003 hidden table state | High | Static LUT extraction passes some captures but fails music. | D-013 contradiction path and stateful-model amendment. |
| R-004 real-time glitch | High | Denormals, allocations, or parameter updates cause dropouts. | T-013…T-016, sanitizers, allocation instrumentation, code review. |
| R-005 DC/unsafe output | High | Mask/table settings produce rail-like output or non-finite values. | Bounded integer pipeline, 5 Hz DC blocker, finite guard, hostile tests. |
| R-006 licence/branding | High | Release is forced down or infringes expectations. | AGPL/commercial decision before release, clean-room assets, G-002 licence review. |
| R-007 host incompatibility | Medium | One DAW restores invalid state or handles buses differently. | pluginval 10, two-host smoke matrix, versioned bounded state parser. |
| R-008 benchmark drift | Medium | CI load causes false performance failures. | Three-run variance check; persistent real misses still block. |
| R-009 platform numeric drift | Medium | Tables/goldens differ across OS/compiler. | Integer table operations, double reference, explicit tolerances, three-OS CI. |
| R-010 range/Fine mismatch | Medium | Control feel differs even with matched tables. | S-010 endpoint measurement; approximation label until verified. |
| R-011 maintenance burden | Medium | Hand-tuned capture/model data cannot be regenerated. | Raw hashes, generator scripts, provenance manifests, knowledge items. |
| R-012 scope confusion | Low | Users expect Eurorack/Pro functions. | Product documentation names original-pedal scope and exclusions. |

**Pre-mortem convergence:** all Critical/High failure modes have a preventive test, gate, or blocking decision rule. Residual risks are Medium/Low only, subject to actual implementation evidence.

**Assumption audit:** A-001…A-003 and A-005…A-006/A-010 are confirmed as scope decisions; A-004 is an environmental fork with an explicit fallback; A-007…A-009 are **unverifiable as hardware facts** and therefore carry R-010/R-002/R-005 controls. No unverifiable premise is silently promoted to a fidelity claim.

## 13. Cold-read handoff checklist

An implementing agent should read §§1–5, then S-000. The plan defines all parameter IDs, numeric conventions, file paths, commands, test tolerances, hardware/approximation forks, gates, failure policies, review requirements, and closing steps. No implementer question is required: missing hardware selects approximation-only; missing repository/credentials blocks S-000; contradictory measurements invoke D-013; invalid frozen tests invoke §10.

**Planning stops here. No plug-in implementation, deployment, or public release has been performed.**
