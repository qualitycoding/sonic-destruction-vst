# Deviations

## D-008 amendment before test freeze

The literal rotate-left/right, bit-reverse, and Gray-code groups in D-008 produced repeated rows. Before any frozen test existed, D-008 was amended to XOR each base row by `(17k mod 256)`. The generator and asset were regenerated. This was a specification correction, not a test weakening; see `lessons/L-20261028T225458Z-rotate-variants-not-unique.md`.
