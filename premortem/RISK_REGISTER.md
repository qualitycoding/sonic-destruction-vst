# Risk Register

The complete R-001…R-012 register is in `plan/PLAN.md` §12. All Critical/High risks have a preventive test, gate, or blocking decision rule. Residual risks are Medium/Low: host compatibility, benchmark drift, platform numeric drift, unverified range/Fine behavior until capture, and maintenance burden.

## v3.9 addendum risk round (2026-11-02)

| Risk | Severity | Mitigation | Status |
|---|---|---|---|
| No independent fresh-context review (D-906) | High | Recorded substitution; independent review scheduled before product execution resumes | Open |
| CI-dependent checks (mutation pricing/floor, matrix jobs) not locally executable (D-905) | Medium | CI is the authoritative venue; estimates labelled; post-conditions gate in CI | Open |
| Lesson/knowledge store push pending G-003 sign-off | Low | Bundle fallback documented; does not block amendment completion | Open |

Pre-mortem rounds repeated to zero Critical risks; High risk retained honestly
with owner and mitigation.

## v3.14 amendment risks (2026-11-07)

| ID | Description | Severity | Likelihood | Root cause | Mitigation |
|---|---|---|---|---|---|
| R-100 | Same-author cold read of the v3.14 changes; independence not demonstrated | High | certain (environment constraint) | No fresh-context reviewer available in sandbox (D-1007) | Recorded as substitution; owner review before merge; verifier + retrospective |
| R-101 | Owner recordings not yet supplied when S-LISTEN becomes runnable | Medium | possible | Capture chain not yet exercised (T-019 floor unknown) | A-904 fallback to approximation-only; G-LISTEN cannot start without reference link records |
