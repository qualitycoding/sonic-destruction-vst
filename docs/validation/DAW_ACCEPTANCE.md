# S-014 DAW Acceptance and Package Rehearsal — Report

- **Date (UTC):** 2026-10-30
- **Stage:** S-014 (plan/PLAN.md §S-014)
- **Commit under test:** `13dff2d` (branch `impl-run-20261028T222647Z-geiger-vst3`)
- **Build:** Debug VST3 rebuilt from that commit (the artefact bundle's inner
  binary had been lost to sandbox `/tmp` recycling; the object tree on the
  persistent mount survived, so only the VST3 link step was redone)

## Environment (recorded per plan)

| Item | Value |
| --- | --- |
| OS | Debian GNU/Linux 12 (bookworm), kernel 5.10.134-18.0.12.lifsea8.x86_64 |
| Display | Xvfb (headless) |
| Validator | pluginval v1.0.4, `--strictness-level 10 --validate-in-process` |

## Smoke matrix

| Check | Method | Result |
| --- | --- | --- |
| Scan | pluginval plugin discovery on bundle | PASS (25 sections, SUCCESS, rc=0) — `docs/validation/pv-s14.log` |
| Load | pluginval instantiate (in-process) | PASS (same run) |
| State save/load | pluginval state round-trips + fuzz parameters | PASS (same run) |
| Rehearsed install | extract `FaultCounter-0.1.0-unsigned-linux-vst3-rehearsal.tar.gz` into clean dir, re-validate extracted copy | PASS — `docs/validation/pv-s14-install.log` |
| Package vs SBOM | tarball listing vs `SBOM.json` declared contents | MATCH: `Fault Counter.vst3` (with moduleinfo.json + Fault Counter.so), `SBOM.json`, `NOTICES.txt`, `INSTALL.txt` |
| WMD trademark/artwork | repo-wide audit | PASS: zero binary artwork/font assets in repo; product identity "Fault Counter"; "WMD" appears only in nominative documentation references |

Package SHA-256 (rehearsal artefact, kept outside the repo on the
persistent mount, not committed):
`e881a0e446f26a1048ea035b9776aa31c735c1d7b45817edea25fa01d8f49e10`

## Blocked items (environment limitations, documented not silent)

| Check | Why blocked | Coverage |
| --- | --- | --- |
| Real DAW load/bypass/automation/render (T-022 host matrix) | No DAWs and no clean test machines in this sandbox | pluginval strictness-10 covers scan/load/state/fuzz headless; CI workflow runs the same validation on GitHub-hosted runners |
| Windows/macOS hosts | Sandbox is Linux-only | CI `pluginval matrix` job covers macOS/Windows runners |
| Release-build rehearsal | Only the Debug tree survived; a Release rebuild exceeds sandbox lifetimes | CI build-test job builds Release; T-015 enforces 20× realtime under NDEBUG |

## Disposition

S-014 is **green within environment limits**: every smoke test executable in
this environment passes, blocked items are explicit and covered by CI. The
done-when clause "every required smoke test passes and package contents match
the SBOM" is met for the Linux/Debug rehearsal; full host-matrix closure is
deferred to CI and to clean machines per the plan's exclusive-resources note.
