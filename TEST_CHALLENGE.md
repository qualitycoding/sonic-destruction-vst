# TEST_CHALLENGE — Frozen routing expectations and golden fixtures

- Date: 2026-10-29 (run `run-20261028T222647Z-geiger-vst3`)
- Branch: `impl-run-20261028T222647Z-geiger-vst3`
- Status: evidence complete, proposed fixes below, applied after this document
- Procedure: plan §10 item 3

## Defect 1 — T-006 "identity table exposes Bits routing directly" (post expectation)

Frozen file: `tests/unit/EngineRoutingTest.cpp` line 59.

Configuration: identity table, Bits 3 (`bitControl = 2/7`), `bitPosition = Post`,
input `fromCode(0xAC)`.

Per plan D-007 the Post chain is:

```text
toCode(fromCode(0xAC)) = 0xAC → identity table → 0xAC → Bits(0xAC, 3)
```

Per plan D-004: `levels = 7`, `q = round(172/255 × 7) = round(4.7216) = 5`,
`Bits = round(5/7 × 255) = round(182.14) = 182 = 0xB6`.

The frozen expectation `fromCode(0xAC)` omits the post-table bit operation and
contradicts D-004/D-007. Verified numerically with the frozen integer formula.

Proposed fix: amend expectation to `fromCode(0xB6)`.

Note: with an identity table both Pre and Post routes yield `0xB6`
(`Bits(0xAC,3)=0xB6` before or after an identity map), so the amended test
still verifies exactly-once application of the bit operation at either
position; route discrimination is covered by the non-reversible table test.

## Defect 2 — T-006 "non-reversible table proves pre and post routes differ" (pre expectation)

Frozen file: `tests/unit/EngineRoutingTest.cpp` line 76.

Configuration: low-nibble table (`c & 0x0F`), Bits 3, `bitPosition = Pre`,
input `fromCode(0xAC)`.

Per plan D-007 the Pre chain is:

```text
toCode(fromCode(0xAC)) = 0xAC → Bits(0xAC,3) = 0xB6 → table: 0xB6 & 0x0F = 0x06
```

The frozen expectation `fromCode(0x0A)` matches no D-004-compliant computation
(it is not the post route either: `table(0xAC) = 0x0C`, `Bits(0x0C,3) = 0x00`).

Proposed fix: amend expectation to `fromCode(0x06)`.

The companion post expectation `fromCode(0x00)` is correct and stays.

## Defect 3 — T-009 golden fixtures contradict their own manifest (`position`)

Frozen files: `tests/golden/expected-*.f32`, `tests/golden/engine-golden-manifest.json`,
generator `tools/tests/generate_engine_golden.py`.

The manifest declares `"position": "post"` and the frozen C++ test
`tests/integration/EngineGoldenTest.cpp` configures `BitPosition::Post`, both
consistent with plan D-007 (bit operation after the table in Post mode).
However the generator computes:

```python
reduced = bits(code, BITS)           # bit operation FIRST
mapped = tables[TABLE * 256 + reduced]
```

That is Pre-table routing, contradicting the manifest label, the frozen test
configuration, and D-007.

Evidence (independent re-computation, float64, vs committed fixtures):

```text
routing used by generator (pre):  max abs diff vs committed goldens = 2.7e-08
correct post routing:             max abs diff vs committed goldens = 1.4e-01
```

The C++ engine output for the impulse fixture at sample 5 is `0.141829789`,
matching the correct Post routing (`0.1418298277` float64); the committed
fixture holds `0.000653357` from the erroneous Pre routing. The engine
implements D-007 correctly; the fixtures and generator are wrong.

Proposed fix: amend the generator to apply the bit operation after the table
lookup for `position: post` (lookup by raw code, then reduce), regenerate all
five fixtures and the manifest, and re-freeze. No change to D-004/D-007, to
the frozen C++ test, or to any implementation source is required.

## Defect 4 — T-006 identity test omits state reset between route measurements

Frozen file: `tests/unit/EngineRoutingTest.cpp`, first test case.

The case renders Pre and then Post through the same `Engine` without
`engine.reset()` between measurements. The DC blocker (D-007) is an IIR whose
state persists across `process` calls, so the Post measurement receives
`coeff × previousOutput` instead of the code-path output "directly", violating
the test's stated intent ("exposes Bits routing directly"). Evidence: actual
`0.42717123 = fromCode(0xB6) × (1 − 2π·5/48000)` exactly.

The sibling test case ("non-reversible table") already calls
`engine.reset()` between routes, showing the intended fixture discipline.
Parameter changes must not reset filter state (D-005), so no engine change is
warranted; `setSettings` correctly preserves state.

Proposed fix: insert `engine.reset();` before the Post measurement in the
identity test case, mirroring the non-reversible case.

## Defect 5 — T-004 1 kHz/280 Hz test constructs a 1-element vector via braced init

Frozen file: `tests/unit/SampleHoldTest.cpp` line 27:

```cpp
std::vector<float> values { size_t (length) };
```

Braced initialization selects `std::initializer_list<float>`, producing a
vector holding **one element** (`200.0f`) instead of 200 zero-initialized
samples. Line 33 then writes `values[i]` for `i = 0…199`, a
heap-buffer-overflow that glibc reports as a `malloc assertion failure in
sysmalloc` at a later allocation (AddressSanitizer pins the write to line 33;
allocation of the 1-element region at line 27). The test could never execute;
the SampleHold implementation is not implicated.

The sibling test file already uses the correct direct-initialization form in
its helper (`std::vector<uint8_t> table (256);` in EngineRoutingTest.cpp).

Proposed fix: amend line 27 to `std::vector<float> values (size_t (length));`.

## Defect 6 — T-004 280 Hz case expects 2 observable held-value changes but the sample-0 capture is unobservable

Frozen file: `tests/unit/SampleHoldTest.cpp` lines 41–50.

The case counts `heldValue()` changes as a proxy for captures. D-005 (plan
text verified) produces captures at samples 0 and 171 for 280 Hz at 48 kHz —
confirmed by exact float64 simulation of the frozen algorithm. However the
capture at sample 0 stores input `0.0f`, which equals the D-005-mandated
initial `held = 0`, so no value change is observable at sample 0; only the
sample-171 capture (held becomes `0.171f`) increments the counter. The frozen
expectation `captureCount == 2` therefore contradicts the test's own
observation method; the implementation conforms to D-005.

Proposed fix: amend the expectation to `captureCount == 1` with a comment
recording that the sample-0 capture is real but unobservable through
`heldValue()`; first-sample capture is already proven observable by the
host-rate sibling case (`process(0.25f, 48000)` returns `0.25`).

## Red-verify and re-freeze

After applying the three fixes: rebuild the frozen suite, confirm 10/10
non-hostile cases pass (T-004 heap assertion tracked separately), regenerate
`tests/FROZEN_MANIFEST.sha256`, cold-read this challenge against plan §10, and
record lesson + knowledge items.

## Amendment round 2 (S-002, 2026-10-30) — frozen T-010/T-011/T-012 compile/runtime defects

Context: the four new S-002 tests (T-001, T-010, T-011, T-012) were frozen on
2026-10-30 after verified red against the S-002 stub. The first green build of
the real JUCE 9 implementation then exposed three further defects in the frozen
test code itself (never compiled before freeze — the JUCE build itself was
blocked until this session). Fixes below; manifest re-frozen.

### Defect 7 — T-012/T-010: invalid BusesLayout construction (undefined behaviour)

Evidence: `juce::AudioProcessor::BusesLayout` default-construction leaves
`inputBuses`/`outputBuses` empty. Mutating through
`getMainInputChannelSet() = ...` is a silent no-op in JUCE 9 (the
`getMain*ChannelSet()` accessors are const-only and return by value), and
`getChannelSet(true, 0)` on an empty array trips `juce_ArrayBase.h:156/157`
assertions and segfaults (observed: SIGSEGV in T-010 bypass case,
`setBusesLayout` jassert at `juce_AudioProcessor.cpp:120`).
Fix: seed every layout from `processor.getBusesLayout()` and mutate through the
non-const `getChannelSet(bool, int)`.

### Defect 8 — T-011: out-of-range-value mutation needle assumed non-JUCE float formatting (OPEN)

Evidence: the "out-of-range parameter value" case searches the payload for the
literal `value="0.73"` and would corrupt it to `value="9.73"`. The serialized
APVTS XML stores float values in JUCE's full-precision form, so the needle is
not guaranteed to match; when it does not match, the mutation is a silent no-op
and the chunk is accepted (observed: restored gain 0.73 where the test expected
rejection). Proposed fix: locate `id="gain"` and corrupt the first digit of the
following `value="..."` attribute instead of matching a fixed literal.
Status: amendment in progress — evidence recorded, fix pending re-verification.

### Defect 9 — BypassTest/StateTest compile errors (fixed pre-freeze, recorded for completeness)

`std::vector<std::vector<float>> expected (size_t (numChannels));` is a vexing
parse (function declaration), and `unsigned char (x)` inside a braced-init-list
is ill-formed. Both fixed before the freeze; the frozen hashes cover the fixed
sources.
