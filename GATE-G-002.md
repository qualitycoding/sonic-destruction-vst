# GATE-G-002 — Public Release Disposition

- **Date (UTC):** 2026-10-30
- **Stage:** S-RELEASE hold record (plan/PLAN.md §S-RELEASE)
- **Gate:** G-002 (public release)

## Human response

**`hold`** — no release. No `proceed: <version> <target>` instruction
with a named version and target has been given, and the plan releases only
after such an explicit instruction.

## Blocking items at hold time

1. **No explicit release approval.** G-002 requires a human `proceed`
   response; absent that, the default is retention of private artifacts.
2. **Licence posture (A-003).** The default open-source path is an
   AGPL-3.0 source release; a closed-source/commercial release requires a
   commercial JUCE licence that does not exist yet. Licence notices are
   drafted in `packaging/NOTICES.md`.
3. **Cross-platform packages.** Only a Linux unsigned rehearsal package
   exists (`packaging/CHECKSUMS.sha256.md` records its provenance and the
   caveat that it predates review fixes). Windows/macOS bundles and a
   rebuilt, post-review Linux bundle are release-time actions.
4. **Real-DAW host matrix.** Deferred items from S-014 (clean machines,
   real DAWs) are covered headless by pluginval and by the CI matrix, but
   a human may want them closed on real hosts before release.

## Evidence bundle (complete and green within environment limits)

- Frozen test suite 34/34; 100k-case fuzz PASS; pluginval strictness 10
  PASS (including installed-package rehearsal); SBOM/package content match;
  code review and adversarial review pass records; CI workflow in place.

## What `proceed` would require

Named version + target, licence decision, rebuilt packages from the
release commit with fresh hashes, and upload performed once per the gate
protocol.
