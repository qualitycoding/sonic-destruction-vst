# Hardware Unavailability Record — S-010

- **Date (UTC):** 2026-10-30
- **Stage:** S-010 "Characterize hardware, or record approximation-only evidence"
- **Plan reference:** plan/PLAN.md §S-010, action 4: "If hardware is absent,
  skip capture without fabricating data and write `research/HARDWARE_UNAVAILABLE.md`."

## Status

The original WMD Geiger Counter pedal and a suitable capture interface are
**not available** in this working environment. No capture chain (pedal,
audio interface, re-amp/DI) exists here, and no raw captures, manifests, or
checksums can be produced.

Per the plan's explicit instruction, this stage is **skipped without
fabricating data**. Nothing in this repository's `hardware/`,
`docs/validation/`, or golden-fixture paths is claimed to be hardware-measured.

## Consequences

1. **Approximation-only status is recorded for the whole project.** Working
   conventions A-007 (sample-rate range 280 Hz–58 kHz) and A-008 (Fine
   mapping) remain unverified by hardware and stay labeled as approximations
   in claims/C-005.
2. **G-001 (hardware fidelity) is gated, not satisfied.** The
   Hardware-Matched track (S-011 table extraction, S-012 chain calibration,
   SC-11 verification T-019…T-021) is **blocked** until physical hardware
   and a capture chain are supplied. This is recorded in GATE-G-001.md.
3. **Software-emulation track continues.** The current sample-rate mapping
   `normalizedToSampleRate(v) = 280 * (58000/280)^v` and all 256-entry tables
   in `tables/` remain the official software emulation; any future hardware
   capture supersedes them through the test-challenge procedure (§10 of the
   QualityCoding protocol), not silently.
4. **No interpolation, no invented codes.** Per D-008 and the S-011 contract,
   no table cell is ever synthesized. All current tables are
   software-generated from published/observed behavior and remain so labeled.

## What would unblock this

- A physical WMD Geiger Counter unit (any firmware revision documented at
  capture time).
- An audio interface with documented gain staging, plus a re-amp/DI chain
  able to hold chain noise below −50 dBFS (plan §S-010 "On failure").
- Fixed capture scripts as specified in the plan; captures stored as
  immutable WAVs with settings manifests and checksums.

## Evidence

- This file is the entire evidence for S-010's "hardware absent" branch.
- Plan checkpoints/capture hashes: not applicable (no captures exist).
