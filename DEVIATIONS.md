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

## S-002 environment recovery (2026-10-30)

GitHub (codeload/github.com/raw) became unreachable mid-session (connections
stalled at 0 B/s). The pinned JUCE 9.0.4 commit was re-obtained from the gitee
mirror `mirrors/JUCE` and verified bit-identical to the GitHub commit by
comparing git blob hashes (`git hash-object`) against Software Heritage's
archive of the pinned commit (e.g. root CMakeLists.txt `6367d548...`, APVTS
header `93c46d35...` — all matched). Dependencies are now consumed via
`FETCHCONTENT_SOURCE_DIR_*` overrides pointing at the extracted local copies;
`cmake/Dependencies.cmake` pins are unchanged. Build trees must live on an
exec-capable filesystem (`/var/tmp`): the `/mnt/agents` fuse mount supports no
exec bits, which breaks `juceaide` at configure time.
