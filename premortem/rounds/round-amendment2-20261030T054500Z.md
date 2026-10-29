# Pre-mortem round: amendment round 2 (S-002 frozen-test defects 7-9)

Date: 2026-10-30

## What could still go wrong
1. The amended corruption needle relies on `id="gain"` appearing before its
   `value` attribute in the serialised XML. JUCE writes attributes in insertion
   order; insertion order is stable for a given JUCE version, and the manifest
   pins the test — acceptable residual risk, documented here.
2. Debug-build JUCE assertions (UTF-8 validity on intentionally-garbage state
   chunks) print to stderr during T-011. They do not fail tests; a
   JUCE_ASSERTIONS_ARE_FATAL build would abort. S-009 validation must treat
   that flag as unsupported for the malformed-chunk test.
3. Block-granularity parameter smoothing (5 ms ramps stepped per block) is
   audible as zipper noise only for blocks >= ~256 samples at 48 kHz. S-008
   UI work should revisit per-sample smoothing if the plan's D-007 demands it.
4. gitee mirror lag: if a future session re-fetches JUCE, the mirror could
   briefly lack the pinned commit; SWH blob-hash verification (or a direct
   github retry) remains the fallback.
