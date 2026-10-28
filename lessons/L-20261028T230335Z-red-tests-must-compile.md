# L-20261028T230335Z-red-tests-must-compile

- **Kind:** test-infrastructure-error
- **Tags:** `tech:cpp`, `tech:testing`, `applies:tdd`
- **Trigger:** The first red-verification build stops in test compilation before exercising stubs.
- **What went wrong:** Missing includes and a vexing-parse declaration made tests fail as source errors.
- **Correction:** Fixed the test declarations and reran the build; red verification is valid only after the harness compiles.
- **Prevention rule:** Before declaring a red run valid, compile the test harness and confirm failures occur in assertions or expected not-implemented exceptions.
- **Detection check:** Every reported failure names a compiled Catch2 test or a stubbed function, never a compiler diagnostic in test code.
- **Evidence:** Pre-freeze red build logs; no frozen assertion changed.
