# Lesson: JUCE 9 API deltas, raw-store state semantics, and sandbox recovery

Date: 2026-10-30 (S-002)

## What happened
First green build of the real JUCE 9 processor exposed both API differences and
frozen-test defects that compile-only verification could never catch.

## JUCE 9 API facts verified against source (juce-framework/JUCE @ 3fb5cbc, v9.0.4)
- No `juce::AudioProcessorParameterListener`; use nested
  `AudioProcessorValueTreeState::Listener` with
  `parameterChanged (const String&, float)`, registered via
  `apvts.addParameterListener (id, this)`.
- `StringFromValue` is `std::function<String (float, int)>`; the int is the
  max-length hint. `AudioParameterIntAttributes` uses `String (int, int)`.
- `ParameterLayout` iterates `std::unique_ptr<RangedAudioParameter>`.
- `AudioProcessorParameterWithID` exposes `paramID` / `getParamID()`, not
  `getParameterID()`.
- `BusesLayout::getMainInputChannelSet()` is const-only and returns **by value**
  in JUCE 9; mutate via `getChannelSet (bool, int)`. A default-constructed
  `BusesLayout` has **no buses** — indexing bus 0 is UB.
- `RangedAudioParameter::convertFrom0to1` denormalises and snaps; the raw
  `NormalisableRange::convertFrom0to1` does not snap.
- APVTS serialises **denormalised** values in state XML
  (`ParameterAdapter::unnormalisedValue`), at full float precision.

## State semantics
`apvts.getRawParameterValue(id)->store(v)` bypasses the parameter object: the
AudioParameter's own value and the APVTS ValueTree are not notified. DSP reading
raw values and `getStateInformation()` must agree — flush raw values into the
copied state tree before serialising, or raw-store callers (and any test doing
the same) silently save defaults.

## Frozen-test defects 7-9 (see TEST_CHALLENGE.md)
- 7: empty-BusesLayout UB in T-010/T-012 (seed from `getBusesLayout()`, mutate
  via `getChannelSet`).
- 8: fixed-literal corruption needle `value="0.73"` does not match JUCE's
  full-precision float formatting — mutation was a silent no-op. Corrupt the
  first digit after locating `id="gain"` instead.
- 9: vexing parse + braced-init functional casts (compile errors, caught
  pre-freeze of the four S-002 tests).

## Environment recovery
- GitHub outage mid-session: pinned JUCE obtained from gitee mirror
  `mirrors/JUCE`, verified bit-identical via `git hash-object` vs Software
  Heritage blob hashes. SWH vault cooking was requested but never left the
  queue — the direct mirror + hash verification path was faster.
- `/mnt/agents` fuse mount: no exec bits (breaks juceaide if the build tree
  lives there) and occasional stale-content reads after writes — verify
  file hashes after copying sources into a build.
- Sandboxes are recycled without warning; /tmp and /var/tmp do not survive.
  Checkpoint the build tree (`rsync -a`) to /mnt/agents after every build
  chunk; restore with rsync and resume with ninja.
