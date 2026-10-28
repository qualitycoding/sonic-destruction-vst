# Geiger Counter–Style VST3 — Planning Handoff

**Run ID:** `run-20261028T222647Z-geiger-vst3`  
**Branch:** `gen-20261028T222647Z-geiger-vst3`  
**Active profile:** `software` (`software.deploys = false`)  
**Plan status:** `READY`

Start with [`plan/PLAN.md`](plan/PLAN.md). It is the self-contained, zero-question execution plan and includes scope, assumptions, decisions, frozen acceptance-test specifications, literal step definitions, human gates, traceability, research, risks, and closing steps.

## Reading order

1. `plan/PROFILE.md`
2. `plan/PLAN.md` §§1–5
3. `plan/PLAN.md` S-000, then subsequent steps as dependencies permit
4. `plan/PLAN.md` §§7–13 for failure rules, gates, traceability, and resumption

## Execution constraints

- Begin with S-000; do not execute implementation during planning.
- Materialize the acceptance tests and verify them red before implementation.
- Do not weaken frozen tests; use the test-challenge procedure in §10.
- Record corrections in `lessons/` and new reusable facts in `knowledge/`.
- Hardware absence selects the approximation-only track; it never justifies fabricated captures or authenticity claims.
- G-001 controls fidelity disposition, G-002 controls public release, and G-003 controls the final knowledge push.
- Independent steps may continue while waiting at a gate only if they do not depend on its outcome.
- Finish with S-RETRO and S-KNOW, then halt.
