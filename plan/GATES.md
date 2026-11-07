
## G-LISTEN — ABX blind listening gate (v3.14)

- **Trigger:** S-LISTEN, after S-ADVERSARIAL and before S-RETRO.
- **Evidence bundle:** ABX pair key record (seeded, base64-keyed); listener instructions
  phrased as a same/different task; demo renders from event files; owner
  pedal-recording reference link records (source/path, author, licence or
  permission statement, date, checksum, `redistributable = true`); reference-chain
  description (mic'd/DI, interface, gain staging, sample rate); frozen quantitative
  metric report; round count.
- **Questions:** per-pair same/different answers; optional free notes on what was
  discriminable (onset, tone, noise, table character); whether defects heard are
  release-blocking.
- **Allowed responses:** `proceed`; `iterate: <notes>` (at most 3 rounds; every
  reported defect becomes a repair task and a re-render before the next round);
  `proceed-with-rescope: <text>`; `stop`.
- **Protocol note:** same/different (ABX), not which-is-real. Rationale: the
  electric-guitar-builder gate saturated in which-is-real format when the DI
  reference carried fewer realism cues than the candidate; recorded guidance is
  ABX and/or a mic'd reference chain.
- **Reference set:** primary source is the owner's own recordings of the owner's
  physical WMD Geiger Counter (Rule 15 owner's-own-material route). External
  redistributable recordings are the fallback; private references require link
  records and never enter the repository or public evidence branch.

## G-003 — split learning stores (v3.9)

Lessons and knowledge are pushed to **separate** stores: the `Lessons`
repository and the `knowledge` repository. G-003 requests the destinations at
plan close; **do-not-push** either store until human sign-off; the documented
bundle fallback applies when sign-off is withheld. Store publication never holds
up the repository amendment's completion record.
