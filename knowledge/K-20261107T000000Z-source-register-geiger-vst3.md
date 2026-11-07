---
id: K-20261107T000000Z-source-register-geiger-vst3
statement: Source register for the Geiger VST3 planning run (run-20261028T222647Z-geiger-vst3): 6 external sources checked, 6 used, 0 corroborating-only, 0 rejected, 0 unreachable; 0 have no stated licence.
status: current
supersedes: []
tags: [subject:geiger-vst3, domain:audio, tech:juce, tech:vst3, phase:research, kind:source-register]
applies_to_version: version-independent
source: { url: "", doi: "", title: "Source register, run-20261028T222647Z-geiger-vst3", locator: "table below", accessed: 2026-11-07, tier: "n/a" }
confidence: verified
derived_from: [C-001, C-002, C-003, C-004]
discovered_by: planner
run: run-20261028T222647Z-geiger-vst3
recorded_at: 2026-11-07T00:00:00Z
---
| # | Source (title, owner) | URL or DOI | Version or date | Accessed | Tier | Outcome | Content (own words) | Claims | Licence or terms (read where) | Permitted use | Access notes |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | Geiger Counter product page, WMD | https://wmdevices.com/products/geiger-counter | current product page | 2026-10-28 | 1 | used | Official control inventory: preamp, tone disable, sample-rate reduction, bit depth, Bits/Mask, 252 hex wave tables, CV, bypass. | C-001, C-002 | Site content, WMD site terms; licence not restated on page (treated as read-only reference) | read only | none |
| 2 | QualityCoding planning-protocol.md v3.4, qualitycoding | https://github.com/qualitycoding/skills/blob/main/protocols/planning-protocol.md | v3.4 at run start; upgraded to v3.14 by this amendment | 2026-10-28 | 1 | used | Planning structure: freeze, gates, pre-mortem, reviews, closing steps. | n/a (process) | Apache-2.0 (repository LICENSE) | read and adapt with attribution | none |
| 3 | JUCE framework, JUCE framework contributors | https://github.com/juce-framework/JUCE | 9.0.4 at commit 3fb5cbc0340758596dac39f7c8de47d562b4896e | 2026-10-28 | 1 | used | VST3/CMake framework and build targets; AGPLv3 or commercial licensing. | C-003, C-004 | AGPL-3.0 or commercial (repository LICENSE) | read; source release default, commercial release needs licence before G-002 | none |
| 4 | JUCE CMake AudioPlugin example, JUCE framework contributors | https://github.com/juce-framework/JUCE/tree/9.0.4/examples/CMake/AudioPlugin | 9.0.4 | 2026-10-28 | 1 | used | Build-target reference for the plug-in CMake layout. | C-003 | AGPL-3.0 or commercial (repository LICENSE) | read | none |
| 5 | pluginval, Tracktion | https://github.com/Tracktion/pluginval | v1.0.4 at commit ed19c2c16b57a6d94db391bea3ef4a80b769d5bf | 2026-10-28 | 1 | used | Headless VST3 validation and strictness range for T-017. | C-003 | GPL-3.0 (repository) with binary-distribution terms noted in repository | read; validation tool only, not linked into product | none |
| 6 | Catch2, catchorg | https://github.com/catchorg/Catch2 | v3.16.1 at commit 08092139210881f01a34e60c5f2fd6afa0ea4024 | 2026-10-28 | 1 | used | Unit-test framework pinned in D-001. | C-003 | BSL-1.0 (repository LICENSE) | read and embed test-only dependency | none |

Notes: hardware-only claims (C-005, C-006, C-007) are gated, not sourced. The
v3.14 amendment itself (qualitycoding/skills @ `ad62f30a4b2a`) was checked as a
source for the amendment text; protocol text is Apache-2.0. Licence fields were
read from the respective repository LICENSE files at the pinned versions. This
register was compiled from `research/SOURCES.md`, `research/claims.json`, and
`research/rounds/`; completeness check: the six rows equal the union of sources
named there. Confidence `verified` per R7 review against the recorded rows.
