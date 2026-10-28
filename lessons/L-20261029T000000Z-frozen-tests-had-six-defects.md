# L-20261029T000000Z — Frozen tests shipped with six latent defects

- Trigger: first green run of the frozen suite during S-003…S-006 implementation
  (4 of 11 cases failed; two more defects found while fixing the first four).
- Mistake: the pre-freeze red verification only proved that stub
  implementations fail; it never proved that the frozen expectations
  themselves were self-consistent with the plan decisions (D-004/D-005/D-007)
  or internally consistent (state resets, observability, vector sizing).
  Six defects shipped into frozen files:
  1. T-006 identity/Post expectation omitted the post-table Bits operation.
  2. T-006 non-reversible/Pre expectation matched no D-004-compliant chain.
  3. T-009 golden generator applied bit reduction pre-table while its
     manifest declared `position: post` and the frozen test used Post.
  4. T-006 identity case measured Post through stale DC-blocker state
     (missing `engine.reset()` between routes; sibling case had it).
  5. T-004 used braced init `std::vector<float> values { size_t (length) }`,
     creating a 1-element vector → heap-buffer-overflow (glibc malloc
     assertion in sysmalloc at a later allocation, misleading diagnosis).
  6. T-004 280 Hz case counted `heldValue()` changes but expected 2, though
     the D-005-mandated sample-0 capture stores `0.0f` equal to the initial
     held value and is unobservable that way.
- Correction: `TEST_CHALLENGE.md` with numeric evidence per defect; amended
  six expectations/fixtures; regenerated `tests/FROZEN_MANIFEST.sha256`; all 11
  frozen cases pass in Debug and ASan+UBSan builds.
- Prevention rule: before freezing, run every frozen test against an
  independent reference implementation of the plan decisions (e.g., the Python
  golden generator pattern) and require the test author to demonstrate each
  expectation by derivation from plan text, not intuition. Any test that
  counts observations must prove the observation is observable.
- Detection check: any frozen failure whose "actual" matches a coherent
  alternate interpretation of the plan decisions triggers a `TEST_CHALLENGE.md`
  before touching implementation code; run the suite under ASan+UBSan before
  attributing memory faults to the implementation.
- Evidence: TEST_CHALLENGE.md defects 1–6; ctest 11/11 green (Debug and
  ASan+UBSan); sha256sum -c tests/FROZEN_MANIFEST.sha256 all OK.
- Tags: domain:audio, tech:cpp, domain:dsp
