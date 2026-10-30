# GATE-G-001 — Fidelity Disposition

- **Date (UTC):** 2026-10-30
- **Stage:** S-013 "Fidelity disposition gate" (plan/PLAN.md)
- **Gate:** G-001 (hardware fidelity)
- **Inputs:** T-019…T-021 reports — **none exist**; hardware-unavailability record: `research/HARDWARE_UNAVAILABLE.md` (S-010, hardware-absent branch).

## Human response

**`approximation-only`**

## Basis

| Item | Status |
| --- | --- |
| Table coverage (S-011) | Not performed — no hardware captures exist |
| Residuals (S-012) | Not performed — no calibration possible |
| Failed fixtures | Not applicable (no fixtures captured) |
| T-019 (chain noise/repeatability) | Not performed |
| T-020 (table extraction) | Not performed |
| T-021 (chain calibration) | Not performed |

No physical WMD Geiger Counter pedal or capture interface is available in
this environment. Per plan §S-010 action 4, the capture branch was skipped
without fabricating data, and S-011/S-012 (which depend on captures) are
blocked rather than approximated.

## Proposed product label

**"Software emulation inspired by the WMD Geiger Counter — not
hardware-matched."** All user-facing material (about dialog, manual,
release notes, repository README) must carry this label until a future
S-010 capture pass, S-011 extraction, and S-012 calibration produce
Hardware-Matched evidence through the frozen test-challenge procedure.

Working conventions pending hardware verification (see plan claims C-005,
C-006, C-007):

- A-007 sample-rate range 280 Hz–58 kHz and the log mapping
  `normalizedToSampleRate(v) = 280 * (58000/280)^v`.
- A-008 Fine-control mapping and knob endpoint feel.
- Analog preamp/tone stage ordering beyond official descriptions.

## Licensing statement

No WMD trademark, logo, artwork, ROM, firmware, or measured hardware data
is bundled with or generated into this project. All DSP tables under
`tables/` are software-generated approximations. The product name and
visual identity are original and do not imply endorsement by or affiliation
with WMD. If a future hardware pass captures measured data, provenance for
every captured cell must be recorded per S-011 before it ships (D-008).

## Downstream branch

- **SC-11 disposition:** upgraded to `approximation-only`; Hardware-Matched
  track (S-011/S-012/T-019…T-021) remains open and blocked on hardware.
- **G-002 (public release):** remains blocked by policy until the
  approximation-only label review and remaining release gates complete;
  this gate alone does not authorize release.
- **Approximation-only development may continue** (S-014 onward), per
  plan §S-013 "On failure".
