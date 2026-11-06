
## v3.9 addendum traceability (2026-11-02)

| Requirement | Artifact | Step | Verification |
|---|---|---|---|
| Gap ledger per planning root | `plan/ADDENDUM-V3.9-GAP.md` | S-ADD-001 | `v39_addendum_verify.py final` |
| Planning-only scope freeze | D-900 in `plan/DECISIONS.md` | S-ADD-001 | verifier + retrospective |
| Mutation three-tier priced CI, aggregator, report score gate | `reviews/MUTATION.md` | S-ADD-080 | verifier + first CI run (pending in CI) |
| Split lesson/knowledge stores | G-003 in `plan/GATES.md`, `.checkpoints/state.json` | S-ADD-010/S-ADD-060 | verifier |
| Full-context checkpoints / zero-context resume | `.checkpoints/state.json` | S-ADD-020 | verifier |
| Assumption audit, reviews | `premortem/ASSUMPTION_AUDIT.md`, `reviews/` | S-ADD-030 | verifier |
| Honest verification, deviations | `DEVIATIONS.md`, `EXECUTION_LOG.md` | S-ADD-040 | verifier |
| Ephemeral protections | `plan/ENVIRONMENT.md` | S-ADD-050 | verifier |
| CI platform matrix | `plan/CI-MATRIX.md` | S-ADD-070 | cold read |
| Cold read / pre-mortem | this file, `premortem/RISK_REGISTER.md` | S-ADD-090 | verifier |

## v3.14 addendum traceability (2026-11-07)

| Requirement | Artifact | Step | Verification |
|---|---|---|---|
| ABX listening gate, owner reference set | `plan/PLAN.md` S-LISTEN, `plan/GATES.md` G-LISTEN | S-V314-060 | cold read + verifier |
| Reference-material policy (owner's own recordings first) | D-1002, A-904/A-905, G-LISTEN evidence bundle | S-V314-060/S-V314-090 | G-LISTEN record |
| `audio = true` / `courseware = false` profile | `plan/PROFILE.md` | S-V314-000/S-V314-070 | verifier |
| Source register (one `kind:source-register` item) | `knowledge/K-20261107T000000Z-source-register-geiger-vst3.md` | S-V314-080 | R7 completeness note in item |
| `project.nature = hobby` | A-903, D-1006 | S-V314-090 | verifier |
| Mutation sharding condition | D-1003 | S-V314-100 | first CI run (pending) |
| Protocol v3.14 normative header | `plan/PLAN.md` title | S-V314-110 | verifier |
| Same-author cold-read substitution risk | D-1007, `premortem/RISK_REGISTER.md` | S-V314-110 | retrospective |
