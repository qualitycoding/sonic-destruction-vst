---
id: K-20261028T222647Z-pluginval-validation
statement: pluginval v1.0.4 supports headless plug-in validation and strictness levels 1 through 10; level 10 is the planned host-validation gate.
status: current
supersedes: []
tags: [subject:geiger-vst3, domain:audio, tech:vst3, phase:research]
applies_to_version: pluginval v1.0.4 at commit ed19c2c16b57a6d94db391bea3ef4a80b769d5bf
source: { url: "https://github.com/Tracktion/pluginval", doi: "", title: "pluginval repository", locator: "tag v1.0.4 / CLI documentation", accessed: "2026-10-28", tier: 1 }
confidence: verified
derived_from: [C-003]
discovered_by: planner
run: run-20261028T222647Z-geiger-vst3
recorded_at: 2026-10-28T22:26:47Z
---
pluginval v1.0.4 (commit `ed19c2c16b57a6d94db391bea3ef4a80b769d5bf`) supports headless
VST3 validation at strictness levels 1-10; T-017 requires exit 0 at strictness level 10.
Source and locator: `plan/PLAN.md` §11 and `research/SOURCES.md`. Confidence: verified for
documented tool facts; no hardware-only inference included.
