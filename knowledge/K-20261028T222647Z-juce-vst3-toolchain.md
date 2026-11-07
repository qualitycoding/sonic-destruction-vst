---
id: K-20261028T222647Z-juce-vst3-toolchain
statement: JUCE 9.0.4 is pinned by commit for the planned CMake VST3 build; licensing must be satisfied before public release.
status: current
supersedes: []
tags: [subject:geiger-vst3, tech:juce, tech:cpp, phase:research]
applies_to_version: JUCE 9.0.4 at commit 3fb5cbc0340758596dac39f7c8de47d562b4896e
source: { url: "https://github.com/juce-framework/JUCE", doi: "", title: "JUCE framework repository", locator: "tag 9.0.4 / repository LICENSE", accessed: "2026-10-28", tier: 1 }
confidence: verified
derived_from: [C-003, C-004]
discovered_by: planner
run: run-20261028T222647Z-geiger-vst3
recorded_at: 2026-10-28T22:26:47Z
---
JUCE 9.0.4 is pinned by commit `3fb5cbc0340758596dac39f7c8de47d562b4896e`, fetched by
commit rather than a floating branch. JUCE is available under AGPL-3.0 and commercial
licences; a closed-source release requires a commercial licence before G-002. Source and
locator: `plan/PLAN.md` §11 and `research/SOURCES.md`. Confidence: verified for documented
tool facts; no hardware-only inference included.
