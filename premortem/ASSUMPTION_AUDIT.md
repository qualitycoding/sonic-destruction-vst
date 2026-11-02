# Assumption and Premise Audit

See `plan/PLAN.md` §12 for the A-001…A-010 audit. Scope decisions are confirmed. Hardware availability is an explicit fork. Sample-rate endpoints, Fine behavior, analog behavior, static-table behavior, and output-protection assumptions are not promoted to hardware facts without capture evidence.

## v3.9 addendum assumption audit (2026-11-02)

| Assumption | Basis | Verdict |
|---|---|---|
| Audited baseline `v3.4` is accurate | Addendum gap audit at rollout | Accepted (verified by final consistency check) |
| Source-tip drift is the rollout's own addendum commit | Commit message + parent chain | Confirmed (D-901) |
| No local build/mutation execution available | Sandbox probe (git transport blocked; API-only writes) | Confirmed; CI substitution recorded (D-905) |
