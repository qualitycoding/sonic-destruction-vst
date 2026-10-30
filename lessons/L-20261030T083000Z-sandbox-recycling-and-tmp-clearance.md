# Lesson: sandbox recycling and /tmp clearance (2026-10-30, S-002/S-008)

## What happened
This project's build was interrupted repeatedly by sandbox recycling: the
execution sandbox is torn down and recreated without warning, sometimes
mid-build. State outside the persistent mount is lost. Two distinct losses
hit us:

1. `/tmp` (and `/var/tmp`) are wiped on recycle. Our toolchain staging
   (cmake/ninja copied to `/tmp` for `chmod +x`, since the persistent mount
   is fuse.portal and carries no exec bits) vanished, and the configured
   build tree under `/var/tmp/fc-build` vanished with it.
2. The persistent mount `/mnt/agents` survives, but serves occasional
   stale-content reads right after writes, and `sed -i` can fail with
   "preserving permissions … No data available".

The first loss cost a full JUCE re-download and re-configure before the
recovery workflow existed.

## Recovery workflow that now survives recycling
- Toolchain staged once into `/mnt/agents/work/toolchain/` (cmake bin+share,
  ninja binary, verified JUCE 9.0.4 source at commit 3fb5cbc, fetched from
  the gitee mirror when GitHub was unreachable and verified bit-identical
  via Software Heritage blob hashes). Restore per session with
  `sh /mnt/agents/work/toolchain/setup-session.sh`, which copies the tools
  into `/tmp` and `chmod +x` them.
- Build on an exec-capable filesystem only: `/var/tmp/fc-build` (the fuse
  mount cannot run `juceaide`). Configure via
  `/mnt/agents/work/toolchain/configure.sh` with pre-seeded FetchContent
  (`FETCHCONTENT_SOURCE_DIR_CATCH2`, `FETCHCONTENT_SOURCE_DIR_JUCE`) so no
  network is needed at configure time.
- After every working chunk, checkpoint the build tree:
  `rsync -a --delete /var/tmp/fc-build/ /mnt/agents/work/build-cache/fc-build/`.
  On a fresh sandbox: restore with the reverse `rsync`, rerun
  setup-session.sh, and ninja incrementally rebuilds only what changed.
- Run long ninja builds under `timeout 400` in chunks so a mid-build recycle
  loses at most one chunk.
- Never trust a write on the fuse mount without verifying: `grep`/`md5sum`
  the file after every edit or copy; a silent no-op `str.replace` and stale
  reads both happened here.

## Rules going forward
- Anything expensive to recreate (toolchain, vendored sources, golden
  fixtures, build trees) lives under `/mnt/agents/work/` with a documented
  restore script; nothing irreplaceable lives in `/tmp`, `/var/tmp`, or the
  system package cache.
- Session-start checklist on any recycled sandbox: restore toolchain →
  restore build cache → verify key file hashes → build.
- Treat sandbox recycling as routine, not exceptional: every long-running
  step must be resumable from the persistent mount alone.
