# DEVIATIONS — v3.9 addendum amendment

**Repository:** `qualitycoding/sonic-destruction-vst` — 2026-11-02.

1. **Source-tip drift (D-901).** Planning source branch tip `f3bf227957353db61ff1a07f3a9d0910aabdd2bb` is the
   rollout phase's addendum publication commit, direct child of audited
   `8716e093903364fbe8f7cac67e6653f9cee91e53`. Implementation branch descends from the tip; nothing newer was
   overwritten.
2. **Git transport unavailable (D-905).** `github.com:443` git transport is
   blocked from this sandbox; all clone-equivalent reads used `codeload` and all
   commits/pushes used the authenticated GitHub Git database API. Remote SHAs
   were verified after every push via the refs API. The addendum's
   push-and-verify cadence was preserved in substance.
3. **No local build/test/mutation execution (D-905).** Estimates in
   `reviews/MUTATION.md` and the CI matrix are marked as estimates pending the
   first CI run; no unrun check is reported as passed.
4. **No fresh-context reviewer (D-906).** Cold read and adversarial review
   recorded as same-author substitutions with High risk, per S-ADD-090.
5. **Learning-store push deferred.** G-003 destinations requested; pushes
   await sign-off (do-not-push until then).

---

# DEVIATIONS — v3.14 addendum amendment

**Repository:** `qualitycoding/sonic-destruction-vst` — 2026-11-07.

1. **Source drift reconciliation (D-1000 companion).** At push time the planning
   branch head was `27a84825` (9 commits ahead of the original audited
   `8716e093`, all from the merged v3.9 addendum). The implementation branch
   `impl-v314-addendum-20261107` was created from the reconciled head, not the
   stale audited commit; recorded in `ADDENDUM-PLAN-V3.14.md`.
2. **Git transport unavailable (same as v3.9 D-905).** All commits/pushes use the
   authenticated GitHub Git database API; remote SHAs verified after every push.
3. **No fresh-context reviewer (D-1007, same pattern as D-906).** The cold read of
   the v3.14 changes is same-author; recorded as High risk, not an independent pass.
4. **Learning-store push deferred.** G-003 destinations for the v3.14 items
   await sign-off; `do-not-push` until then.
