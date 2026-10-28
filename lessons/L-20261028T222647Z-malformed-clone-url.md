# L-20261028T222647Z-malformed-clone-url

- **Kind:** command-error
- **Tags:** `tech:git`, `applies:planning`
- **Trigger:** A Git clone command fails before creating the destination because the repository URL was mistyped.
- **What went wrong:** The first clone attempt duplicated the owner segment in the repository URL.
- **Correction:** Retried with the exact user-supplied repository URL and verified that the empty repository cloned successfully.
- **Prevention rule:** Before every Git network command, construct the URL directly from the user-supplied owner/repository pair and compare it character-for-character.
- **Detection check:** `git ls-remote <exact-url>` exits 0 before cloning.
- **Evidence:** Planning-session command log; no credentials or personal data recorded.
