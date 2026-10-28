# Deviations

## D-008 amendment before test freeze

The literal rotate-left/right, bit-reverse, and Gray-code groups in D-008 produced repeated rows. Before any frozen test existed, D-008 was amended to XOR each base row by `(17k mod 256)`. The generator and asset were regenerated. This was a specification correction, not a test weakening; see `lessons/L-20261028T225458Z-rotate-variants-not-unique.md`.

## Frozen-test amendment round (S-003…S-006, 2026-10-29)

First green run exposed six defects in frozen tests/fixtures, not in the
implementation: two T-006 expectations contradicting D-004, a T-006 state-reset
omission, a T-004 vector-construction heap overflow, an unobservable-capture
expectation in T-004, and T-009 golden fixtures generated pre-table while the
manifest declared `post`. All handled through `TEST_CHALLENGE.md` per plan §10;
plan decisions D-004/D-005/D-007 unchanged; manifest re-frozen. See lesson
`L-20261029T000000Z-frozen-tests-had-six-defects`.
