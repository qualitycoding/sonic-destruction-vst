# L-20261028T225458Z-rotate-variants-not-unique

- **Kind:** wrong-assumption
- **Tags:** `domain:dsp`, `applies:planning`
- **Trigger:** Generating the procedural tables reports fewer than 28 unique rows in the rotation/reversal family.
- **What went wrong:** D-008's rotate-left/right, bit-reverse, and Gray-code groups contain repeated 256-byte rows.
- **Correction:** Amended D-008 before test freeze to XOR each base row by `(17k mod 256)` and regenerated the asset.
- **Prevention rule:** Before freezing generated fixtures, assert the required number of unique rows for every generator family.
- **Detection check:** `len(set(rows)) == 28` for each of the nine generator families.
- **Evidence:** Pre-freeze generator failure and amended D-008; no frozen test was modified.
