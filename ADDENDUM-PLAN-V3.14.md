# sonic-destruction-vst planning-protocol phase addendum — v3.4 → v3.14

**Status:** READY FOR PLANNING AMENDMENT ONLY  
**Target repository:** `qualitycoding/sonic-destruction-vst`  
**Audited planning source:** `gen-20261028T222647Z-geiger-vst3`  
**Original audited source commit:** `8716e093903364fbe8f7cac67e6653f9cee91e53`  
**Reconciled source commit:** `27a848254913a1b14432caf781aeaa28e7b489c3` (current head of `gen-20261028T222647Z-geiger-vst3` at push time). Drift check found the branch 9 commits ahead, all from the merged v3.9 planning addendum (`ADDENDUM-PLAN-V3.9.md`, gap ledger, status log, reviews, licence, blind-pair scripts); the original audited commit is a strict ancestor. Per the rollout protocol the write was re-based onto the reconciled head after recording this reconciliation, not onto the stale audited commit.
**Baseline protocol:** v3.4, per `plan/PLAN.md` line: “Planning basis: QualityCoding `planning-protocol` v3.4”  
**Target protocol:** v3.14, `qualitycoding/skills` commit `ad62f30a4b2a3287d05289310d867bdd11563fbc`  
**Active profiles and flags:** `software`; `audio = true`; `courseware = false`; preserve base-plan `software.deploys = false` unless a later human gate approves release work.  
**Planning root:** `plan`  
**Owner-supplied hardware:** the owner states they hold the physical original WMD Geiger Counter pedal and can record samples if reference recordings cannot be found. This resolves base-plan A-004 toward the hardware-capture path, subject to the T-019 capture-chain noise floor (≤ −50 dBFS). Owner-made recordings of the owner's own device are the first-choice reference set under Rule 15 (owner's own material, redistributable route), pending the owner's licence statement for those files.  
**Product scope:** unchanged. This plan amends planning only. It does not implement the Geiger/Fault Counter plug-in, change DSP behaviour, weaken frozen tests, merge branches, release, deploy, publish, or execute product steps.

## Reading order

1. This addendum.
2. `ADDENDUM-PLAN-V3.9.md`, where present, because it already records the v3.5–v3.9 gap ledger.
3. `plan/ADDENDUM-V3.9-GAP.md`, where present.
4. Base `plan/PLAN.md` and its appendices.
5. `HANDOFF.md`.

Where this addendum and the v3.9 addendum overlap, this addendum does not rescope the product; it supersedes only the protocol-version bookkeeping by extending the target from v3.9 to v3.14. The v3.5–v3.9 phases below are included because the request is “since the plan was created”; the net-new phases for the current repository are v3.10–v3.14.

## Phase ledger added since the sonic-destruction-vst base plan

| Protocol phase | Added capability | Applicability to sonic-destruction-vst | Required planning amendment |
|---|---|---|---|
| v3.5 | Honest verification, command/pin evidence, deviation completeness | Applicable | Harden every check so it can fail; add artifact post-conditions; verify every literal command/pin against the pinned version; log every substitution in `DEVIATIONS.md`. |
| v3.6 | Environment-class probe, ephemeral execution protections, immediate push verification after every successful chunk | Applicable | Record stable/ephemeral classification; use durable-mount discipline; make long work checkpointed and CI-bound where local execution is ephemeral; push and verify remote SHA after every successful chunk. |
| v3.7 | Split Lessons and knowledge stores | Applicable | Treat `qualitycoding/Lessons` as the lesson store and `qualitycoding/knowledge` as the knowledge store; read and push them separately; never combine them in G-003/S-KNOW. |
| v3.8 | Intake-derived CI platform matrix with explicit coverage or waivers | Applicable | Map every supported OS/architecture/runtime bound to a versioned CI job or a `D-###` waiver; no `-latest` runner labels; include ARM64/libc-variant/oldest-toolchain considerations where applicable. |
| v3.9 | Priced three-tier mutation CI with one aggregator and report-based score gate | Conditionally applicable | The base plan currently opts out of mutation testing in D-009. Either retain that opt-out as a reviewed `D-###`, or add `S-MUTATE` with pricing, triggers, report-file score gate, minimum mutant post-conditions, and an always-running aggregator. |
| v3.10 | Audio listening validation: blind A/B reference comparisons and demo listening at a human gate | Applicable because this is audio work | Add `S-LISTEN` before `S-RETRO`; define reference set under Rule 15, frozen pass condition, blind-pair seed rule, demo event renders, CI evidence branch, and `G-LISTEN`. |
| v3.11 | Learning-exercise pathway for scored learner material | Not applicable | Record `courseware = false` in `plan/PROFILE.md`; no `S-LEARN` or `G-LEARN` is required. Add only if the deliverable changes to scored learning material. |
| v3.12 | Source register: every external source checked is listed with content, licence/terms, permitted use, and outcome | Applicable | Add research `R7`; end each research round with `Sources checked`; produce one `kind:source-register` knowledge item; verify completeness before G-003. |
| v3.13 | Project nature and reference-material route; redistributable-first references with private-reference controls | Applicable, especially for audio references | Record `project.nature = hobby`; prefer redistributable references; use private references only with link records, checksums, licence/terms evidence, no republishing, and fail-closed checks. |
| v3.14 | Mutation sharding when mutation testing exceeds runtime, memory, or install budget | Conditionally applicable | If `S-MUTATE` is enabled and the priced job exceeds budget, design a generated per-file/per-module shard matrix with CPU/memory/parallelism/step caps, bounded three-attempt installs, per-shard report gates, and one completeness-checking aggregate job. If mutation remains opted out, record that this phase is N/A by decision. |

## Amendment steps

### S-V314-000 Verify source, target protocol, and repository state

- **Tier:** Haiku
- **Depends on:** none
- **Inputs:** this addendum; base `plan/PLAN.md`; existing `ADDENDUM-PLAN-V3.9.md`; `plan/ADDENDUM-V3.9-GAP.md`; `qualitycoding/skills` protocol v3.14 at `ad62f30a4b2a3287d05289310d867bdd11563fbc`.
- **Actions:**
  1. Confirm the working branch is the audited sonic planning branch or a new amendment branch created from `8716e093903364fbe8f7cac67e6653f9cee91e53`.
  2. Confirm the target protocol title is v3.14 and the review record is present.
  3. Record that the base plan was created at v3.4 and that v3.5–v3.9 already have a repository addendum if `ADDENDUM-PLAN-V3.9.md` is present.
  4. Create or update `plan/PROFILE.md` with active profile `software`, mode flag `audio = true`, mode flag `courseware = false`, and explicit N/A entries for conditional phases that do not apply.
- **Outputs:** updated `plan/PROFILE.md`; amendment status entry in `.checkpoints/state.json`.
- **Done when:** source commit, baseline v3.4, target v3.14, and all applicability decisions are recorded; no credential or token appears in any file.
- **Checkpoint:** source/target commit, profile record, artifact hashes.
- **On failure:** halt with `BLOCKED.md`; do not overwrite newer planning work.
- **Gate:** none.

### S-V314-010 Reconcile the v3.5 honest-verification phase

- **Tier:** Sonnet
- **Depends on:** S-V314-000
- **Actions:**
  1. Audit `plan/PLAN.md`, `plan/ENVIRONMENT.md`, CI references in the plan, and any workflow templates for failure-swallowing idioms.
  2. Ensure every load-bearing check can fail visibly; bounded retries must end in visible failure.
  3. Ensure every producing step names artifact existence and a parse/sanity post-condition.
  4. Ensure every numeric performance target names its runner class and has a pre-decided miss rule.
  5. Verify every literal command, flag, config file, and version pin against the pinned tool version, recording spike output or pinned-version documentation locators.
  6. Ensure `DEVIATIONS.md` exists and is named as the substitution log.
- **Outputs:** updated `plan/ENVIRONMENT.md`, corrected plan/workflow text, `DEVIATIONS.md` policy.
- **Done when:** no unverifiable command/pin remains in the planning contract; every acceptance metric gates; every substitution route is explicit.
- **Gate:** none.

### S-V314-020 Add the v3.6 ephemeral-environment and push-cadence phase

- **Tier:** Sonnet
- **Depends on:** S-V314-010
- **Actions:**
  1. Record the environment-class probe in `.checkpoints/state.json`; unknown local/containerised environments default to ephemeral.
  2. Add durable-mount-only rules for long work, turn-start restaging, executable-bit restoration, checkpointed builds, and dependency-ordering invariants.
  3. Right-size local work to container lifetimes; move ASan/UBSan fuzzing, long benchmarks, and heavy capture jobs to named CI jobs where local execution cannot complete.
  4. Restate Global Rule 6 in `HANDOFF.md`: commit and push after every successful chunk, then compare `git ls-remote` with local HEAD; retry transient failures with backoff; report mismatches.
- **Outputs:** updated `HANDOFF.md`, `.checkpoints/state.json`, resilient long-step text in `plan/PLAN.md`.
- **Done when:** every long step names checkpoint/resume/durable state and every successful chunk has an immediate push plus remote-SHA verification rule.
- **Gate:** none.

### S-V314-030 Add the v3.7 split lesson/knowledge store phase

- **Tier:** Sonnet
- **Depends on:** S-V314-020
- **Actions:**
  1. Set lesson store to `qualitycoding/Lessons` and knowledge store to `qualitycoding/knowledge` in intake assumptions and checkpoint state, unless the human has supplied different stores.
  2. Update research intake language so lessons are matched from the lesson store and facts from the knowledge store using their separate matching rules.
  3. Update `G-003`/`S-KNOW` so lessons and knowledge are pushed separately, each index regenerated, each remote retry limit counted separately, and each store has a separate bundle fallback.
  4. Ensure no plan text treats lessons and knowledge as one combined destination.
- **Outputs:** updated `plan/ASSUMPTIONS.md`, `research/PRIOR_KNOWLEDGE.md`, `plan/GATES.md`, `HANDOFF.md`, and closing-step text.
- **Done when:** both stores are named independently and G-003 has separate push/index/retry/fallback paths for each store.
- **Gate:** none.

### S-V314-040 Add the v3.8 CI platform matrix phase

- **Tier:** Sonnet
- **Depends on:** S-V314-030
- **Actions:**
  1. Derive the CI matrix from the supported platforms declared in the base plan: Windows 10+ x64, macOS 11+ universal, and Linux x64 VST3.
  2. Replace any `-latest` runner label with versioned labels and record existence checks for each label/action/tool pin.
  3. Add default extras or explicit waivers: ARM64 Linux where support is claimed; one libc-variant container job where native code makes it meaningful; one oldest-supported-toolchain job; mobile VM/emulator jobs are N/A unless intake adds Android/iOS.
  4. Map every supported platform and architecture in `plan/TRACEABILITY.md` to a CI job or `D-###` waiver.
  5. Keep mutation jobs out of the per-platform matrix.
- **Outputs:** CI matrix in `plan/ENVIRONMENT.md`, platform mapping in `plan/TRACEABILITY.md`, runner/action existence evidence, workflow or plan updates.
- **Done when:** every intake-declared platform maps to a versioned CI job or waiver; `fail-fast: false`; no required workflow uses a `paths:` filter to skip its own status.
- **Gate:** none.

### S-V314-050 Add the v3.9 mutation phase, or keep an explicit opt-out

- **Tier:** Sonnet
- **Depends on:** S-V314-040
- **Actions:**
  1. Review base decision D-009, which currently opts out of mutation testing.
  2. If the opt-out remains, restate it as a current protocol `D-###`: mutation is omitted because the DSP correctness gate combines golden vectors, sanitizer fuzzing, pluginval, hardware capture evidence, and adversarial review; include the re-entry condition for reconsidering mutation testing.
  3. If mutation is enabled, add `S-MUTATE`: pin the mutation tool and config; scope mutable production modules; record fast-test list and omissions; measure baseline test time and mutant count; estimate full/incremental runtime against a stated budget; define three triggers — changed-file incremental on PR/default push, scheduled full run with named owner/notification, and workflow_dispatch; add one always-running aggregator as the only required mutation status; compute score from the tool report, not exit status; require report existence and minimum mutant count; demonstrate once that below-floor fails; set the floor five points below the first real measured score.
- **Outputs:** `reviews/MUTATION.md` either as a formal opt-out record or as priced mutation evidence; updated `plan/DECISIONS.md`; workflow text if enabled.
- **Done when:** mutation is either fully specified with pricing/triggers/aggregator/report gate or explicitly opted out by a reviewed `D-###`.
- **Gate:** none.

### S-V314-060 Add the v3.10 audio listening validation phase

- **Tier:** Opus
- **Depends on:** S-V314-050
- **Actions:**
  1. Mark `audio = true` in `plan/PROFILE.md`.
  2. Define the reference set under Rule 15. Primary source: owner-recorded samples of the owner's own physical WMD Geiger Counter (owner's own material, redistributable route); request the owner's statement of licence/permission for those files and record each with URL/path, author, licence or terms, date accessed, checksum, and `redistributable = true`. Fall back to a redistributable-first external search only if the owner recordings are unavailable or insufficient; only then consider private references with link records, checksums, licence/terms evidence, and `redistributable = false`; never commit private reference audio or derived pairs.
  3. Add quantitative frozen thresholds for the reference comparison; the listening gate must not relax them.
  4. Add `S-LISTEN` after implementation steps and `S-REVIEW`/`S-ADVERSARIAL`, and before `S-RETRO`.
  5. In `S-LISTEN`, specify: blind-pair builder and seed rule; at least one pair set cut from the start of each clip; fixed pass condition such as maximum correct picks out of 12; demo list as event files rendered headlessly; CI marker commit `[gate-evidence <gate-id>]`; orphan evidence branch; delivery URLs; iterate procedure with at most three rounds.
  6. Use the **same/different (ABX) protocol**, not which-is-real: for each pair the listener answers "same" or "different", scored against chance on discriminating candidate render from the owner pedal recording. Rationale: a which-is-real format saturates once the candidate matches or exceeds the reference in realism cues, and a previous qualitycoding gate recorded reversed discrimination (DI reference judged less realistic than the synth) as the reason to switch formats. Also specify the reference chain for owner captures (mic'd amp/cab or DI with documented chain) so the reference itself carries full realism cues.
  7. Define `G-LISTEN` in `plan/GATES.md` with responses `proceed`, `iterate: <notes>`, `proceed-with-rescope: <text>`, and `stop`.
- **Outputs:** `plan/GATES.md`, `S-LISTEN` in `plan/PLAN.md`, reference link records, frozen listening metrics, evidence-branch plan.
- **Done when:** listening phase has references or an explicit waiver, frozen pass condition, blind-pair seed rule, demos, CI evidence branch, and a human gate.
- **Gate:** G-LISTEN is defined here; execution waits at it when reached.

### S-V314-070 Record the v3.11 courseware phase as N/A

- **Tier:** Haiku
- **Depends on:** S-V314-060
- **Actions:**
  1. Record `courseware = false` and the reason: this deliverable is a VST3 audio effect, not scored practice material.
  2. State that `S-LEARN` and `G-LEARN` are N/A and produce no artifacts.
  3. Add a profile-change rule: if the deliverable later becomes scored learning material, re-open intake, activate `courseware`, and add the learning pathway before continuing.
- **Outputs:** N/A record in `plan/PROFILE.md`.
- **Done when:** courseware pathway is explicitly disabled and no cold-read/final check requires it.
- **Gate:** none.

### S-V314-080 Add the v3.12 source-register phase

- **Tier:** Sonnet
- **Depends on:** S-V314-070
- **Actions:**
  1. Add research step `R7` to the planning/implementation research loop: compile every external source checked from round logs, `research/SOURCES.md`, `research/claims.json`, novelty searches, command/pin verification, and spike downloads.
  2. For each source, record title, URL/DOI, owner/author, version/date, date accessed, tier, outcome, content summary in the agent’s own words, supported/contradicted claims, licence/terms read from the source, permitted use, compatibility notes for reused material, and access notes.
  3. Spot-check at least five rows against live sources; fix discrepancies and re-check further rows until a pass finds none.
  4. Run a mechanical completeness check that the register contains the union of all checked sources and that every row has every required field.
  5. Produce one `kind:source-register` knowledge item and carry it to G-003.
- **Outputs:** `knowledge/K-<timestamp>-source-register-<slug>.md`; updated research round template.
- **Done when:** exactly one source-register item exists, passes review and completeness checks, and is named in the closing gate record.
- **Gate:** none.

### S-V314-090 Add the v3.13 project-nature and reference-material phase

- **Tier:** Sonnet
- **Depends on:** S-V314-080
- **Actions:**
  1. Record `project.nature = hobby` in `plan/ASSUMPTIONS.md`.
  2. Add the redistributable-first search order for all reference material: owner’s own material (here, the owner’s own pedal recordings — record the owner’s licence/permission statement for them), public-domain/CC0, then permissive/attribution licences; log what was searched.
  3. Add private-reference controls: use only when redistributable material cannot meet the need; read licence/terms from the original source; record URL, author, licence/terms URL, version/date, date accessed, and sha256; download on demand outside the working tree; never commit or publish the file or derived material; fail closed if the checksum or licence flag is missing.
  4. Add a check that any step capable of publishing reference-derived material verifies the `redistributable` flag and fails when a private reference would be published.
  5. Ensure `S-LISTEN` and any future audio evidence bundle obey this phase.
- **Outputs:** reference-material policy in `plan/DECISIONS.md`, link-record template, fail-closed check.
- **Done when:** every reference has a route (`redistributable` or justified private reference), and no private reference can enter a public artefact.
- **Gate:** none.

### S-V314-100 Add the v3.14 mutation-sharding phase conditionally

- **Tier:** Sonnet
- **Depends on:** S-V314-050 and S-V314-090
- **Actions:**
  1. If mutation testing remains opted out, record v3.14 mutation sharding as N/A by D-ID and require re-evaluation only if D-009 is reopened.
  2. If mutation testing is enabled, check the priced job against runtime, memory, and install budgets.
  3. If the job exceeds budget, follow `skills/mutation-sharding`: generate a per-file or per-module shard matrix with CPU, memory, parallelism, and step limits; bound tool installation to three timed attempts; add per-shard report gates; upload every shard report; and make one aggregate job verify expected report completeness before scoring.
  4. Ensure the aggregate job, not any individual shard, is the only required mutation status.
- **Outputs:** sharding decision in `plan/DECISIONS.md`; generated shard matrix if applicable; updated mutation workflow.
- **Done when:** mutation is either opted out, priced within budget without sharding, or sharded with complete report gates and an aggregate completeness check.
- **Gate:** none.

### S-V314-110 Update traceability, cold read, pre-mortem, and final consistency checks

- **Tier:** Opus
- **Depends on:** S-V314-010 through S-V314-100
- **Actions:**
  1. Update `plan/TRACEABILITY.md` so every new protocol requirement maps to an artifact, step, and verification.
  2. Update the `plan/PLAN.md` status header to name protocol v3.14 and include this addendum in the normative planning contract.
  3. Run a fresh-context cold read for zero-question execution and a mechanical check for referenced paths, IDs, N/A sections, forbidden failure-swallowing idioms, platform mappings, audio listening gate, source register, and split-store closing semantics.
  4. Run one pre-mortem round over the changed planning phases; resolve all Critical/High risks or mark the plan blocked.
  5. Recompute frozen manifests only where a legitimate planning amendment changed a frozen artifact; never change expected product behaviour.
- **Outputs:** updated traceability, status header, cold-read record, pre-mortem record, verifier output.
- **Done when:** no unresolved cold-read items remain, pre-mortem has no unresolved Critical/High risks, and the final consistency checklist passes.
- **Gate:** none.

### S-V314-120 Close the planning amendment and persist learning

- **Tier:** Haiku
- **Depends on:** S-V314-110
- **Actions:**
  1. Write a short amendment retrospective naming the v3.5–v3.14 additions, whether each is active, conditional, or N/A for this repository, and any substitutions made while planning.
  2. Mark all amendment steps pass with commits and evidence; no pending, failed, or unapproved blocked row may remain.
  3. Commit and push the amendment branch, verify remote SHA equals local HEAD, and report branch/SHA.
  4. At G-003, request separate lesson and knowledge destinations and push only after sign-off; otherwise write separate bundle fallbacks.
- **Outputs:** amendment retrospective, completed status rows, remote branch evidence, G-003 record.
- **Done when:** all applicable amendment steps pass, remote matches local, product execution remains unauthorised, and learning-store publication is recorded.
- **Gate:** G-003 for learning stores only; no product release gate is triggered by this planning amendment.

## Human gates affected by this addendum

| Gate | Trigger | Evidence bundle | Allowed responses |
|---|---|---|---|
| G-LISTEN | After `S-LISTEN` when audio implementation evidence exists | ABX pair key record, listener instructions (same/different task), demo renders, owner pedal-recording reference link records with licence/permission statement and checksums, reference-chain description (mic'd/DI), metric report, round count | `proceed`; `iterate: <notes>`; `proceed-with-rescope: <text>`; `stop` |
| G-003 | Closing learning-store push | Lesson IDs and knowledge IDs, including the source register, counts by outcome, counts of `unknown`/`none stated` licences, secret-scan result | `push-to-proposed`; `push-to: <lesson-target> [knowledge-target]`; `do-not-push` |

Existing product gates G-001 and G-002 remain unchanged and are not advanced by this planning amendment.

## Final consistency checklist for this amendment

- `plan/PROFILE.md` says `software`, `audio = true`, `courseware = false`, and records all conditional phases.
- Honest verification is active: no load-bearing swallowed failure, every artifact has post-conditions, every numeric threshold gates.
- Ephemeral rules and immediate push cadence are present.
- Lesson and knowledge stores are split.
- CI platform matrix covers Windows x64, macOS universal, and Linux x64, with explicit versioned labels or waivers.
- Mutation is either fully priced and gated, or explicitly opted out with a re-entry condition.
- `S-LISTEN` and `G-LISTEN` are present because the deliverable is audio, using the ABX protocol with owner pedal recordings as the primary reference.
- Courseware is explicitly N/A.
- Source register and project-nature/reference-material controls are present.
- Mutation sharding is addressed conditionally.
- Cold read, pre-mortem, traceability, and final mechanical verification pass.

**Planning stops here. Do not execute the product plan. Do not release, deploy, publish, or submit anything.**
