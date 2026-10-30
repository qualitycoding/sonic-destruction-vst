# Rehearsal package — Fault Counter v0.1.0 (unsigned, Linux VST3)

Artefact branch. This tarball is the S-014 packaging rehearsal package; it is
intentionally kept off the implementation branch (`impl-run-20261028T222647Z-geiger-vst3`).

- Built from commit `13dff2d` — BEFORE the S-REVIEW/S-ADVERSARIAL fixes
  (`ab508b8`, `403bff6`). Do not distribute; a release build must be rebuilt
  from the release commit at G-002 time.
- Debug build, unsigned, Linux x86_64 only. SHA-256 in `package-sha256.txt`:
  `e881a0e446f26a1048ea035b9776aa31c735c1d7b45817edea25fa01d8f49e10`
- Contents: `Fault Counter.vst3` (with `Fault Counter.so` + `moduleinfo.json`),
  `SBOM.json`, `NOTICES.txt`, `INSTALL.txt`.
- Gate state: G-002 hold (see `GATE-G-002.md` on the impl branch).
- Extraction + pluginval strictness-10 validation of the installed copy:
  PASS (`docs/validation/pv-s14-install.log` on the impl branch).
