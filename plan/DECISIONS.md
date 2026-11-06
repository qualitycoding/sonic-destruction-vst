
## Addendum v3.9 decisions (2026-11-02)

- **D-900** — Planning-only amendment. Product scope, public interfaces,
  expected test behaviour, performance targets and human gates do not change,
  except the mandatory split G-003 lesson/knowledge store semantics.
- **D-901** — Source-branch drift: the planning source branch tip `f3bf22795735`
  is the rollout phase's `plan: add protocol v3.9 addendum` commit, a direct
  child of the audited source commit `8716e0939033`. The implementation
  branch therefore descends from `f3bf22795735`, which itself descends from the
  audited commit; no newer planning was overwritten or rebased away.
- **D-902** — No platform expansion: no new OS/architecture/runtime support is
  claimed; CI coverage is recorded as plan-level jobs/waivers only.
- **D-903** — No release, deployment or package publication is authorised or
  planned by this amendment.
- **D-904** — Ambiguities resolved from existing plan artifacts and repository
  evidence only; defaults follow the addendum's no-added-platform / explicit
  waiver rule.
- **D-905** — Local build and test execution is not available in the ephemeral
  sandbox; the CI matrix jobs added to the plan are the authoritative execution
  venue (honest-verification substitution, see DEVIATIONS.md).
- **D-906** — Fresh-context independent review is unavailable in this
  environment; the cold read is recorded as a substitution with High risk
  (S-ADD-090), not claimed as an independent pass.
- **D-980** — Mutation testing design: see `reviews/MUTATION.md` (priced
  three-tier trigger strategy, one aggregator job, report-based score gate).

## Addendum v3.14 decisions (2026-11-07)

- **D-1000** — Planning-only amendment, v3.4 → v3.14. Product scope, public
  interfaces, frozen expected behaviour, performance targets and existing human
  gates are unchanged; the amendment adds the v3.10–v3.14 protocol capabilities
  (v3.5–v3.9 already covered by the executed v3.9 addendum).
- **D-1001** — Listening protocol is **same/different (ABX)**, not
  which-is-real. Rationale: electric-guitar-builder G-LISTEN round 4 showed the
  which-is-real format saturating (listener chose the candidate as real in 7/12
  against a DI reference); the owner-recorded guidance is ABX and/or a mic'd
  reference chain. The reference-chain description is part of the G-LISTEN
  evidence bundle.
- **D-1002** — Primary listening reference set: the owner's own recordings of
  the owner's physical WMD Geiger Counter (Rule 15 owner's-own-material,
  redistributable route). The owner states the physical device is available for
  captures if references cannot be found; this resolves A-004 toward the
  hardware-capture path subject to the T-019 noise floor. Each file needs the
  owner's licence/permission statement, checksum, and link record.
- **D-1003** — Mutation sharding (v3.14): N/A at planning time. The v3.9-priced
  design stays within the 20 min incremental / 120 min full budgets as an
  estimate; sharding activates only if the first real CI run exceeds runtime,
  memory, or install budget, per `skills/mutation-sharding`.
- **D-1004** — `courseware = false`: deliverable is a VST3 audio effect, not
  scored practice material; §3.11 produces no artifacts. Reactivate only through
  a profile-change D-ID if the deliverable changes.
- **D-1005** — Source register (v3.12): compiled as one
  `kind:source-register` knowledge item
  (`knowledge/K-20261107T000000Z-source-register-geiger-vst3.md`) from
  `research/SOURCES.md`, `research/claims.json`, and round logs; it travels to
  the knowledge store at G-003.
- **D-1006** — `project.nature = hobby` (v3.13): reference material follows the
  redistributable-first route; restrictive material may be a linked private
  reference only, never committed or republished.
- **D-1007** — Same-author cold-read substitution (as D-906): no fresh-context
  reviewer available in this sandbox; recorded as High risk, not an independent
  pass.
