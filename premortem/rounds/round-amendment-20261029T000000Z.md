# Pre-mortem round — post-amendment (§10 item 3)

- Date: 2026-10-29, run `run-20261028T222647Z-geiger-vst3`
- Trigger: frozen-test amendment round (TEST_CHALLENGE.md, six defects)

## Question asked

"Six frozen defects slipped through red verification. What else in the current
state is likely wrong in the same ways?"

## Failure modes imagined and checks run

1. **More braced-init/vector-sizing traps in remaining frozen tests or
   implementation.** Checked all `std::vector` constructions in `tests/` and
   `src/`; only the one T-004 instance existed (fixed). ASan+UBSan full suite
   run: clean.
2. **Golden fixtures still inconsistent with the manifest in another
   parameter.** Verified the manifest's sampleRate/requestedRate/bits/table/
   position/length against the generator constants (single source: generator
   now emits `POSITION` into the manifest) and against the frozen C++ test
   configuration (48 kHz, 9 kHz, 3 bits, table 42, Post, 512 samples) — all
   match.
3. **C++ engine diverges from D-004/D-005/D-007 in a path no frozen test
   covers.** T-009 exercises post-route; T-006 covers both routes with two
   tables; T-002 covers Bits/Mask edge depths; T-003 covers phase timing;
   residual gap: Mask-mode golden coverage and pre-route golden coverage —
   accepted as S-009 hardening backlog, recorded in EXECUTION_LOG.md.
4. **Stale hashes elsewhere** (checkpoints state.json artifact map vs disk).
   Recomputed after re-freeze; updated in this round.
5. **The amendments themselves introduced a wrong expectation.** Each amended
   value was derived numerically from plan D-004/D-005/D-007 text (see
   TEST_CHALLENGE.md); identity-table Pre==Post symmetry sanity-checked.

## Outcome

No new blocking findings. Residual risk: golden coverage of Mask mode and
pre-route paths deferred to S-009 hardening.
