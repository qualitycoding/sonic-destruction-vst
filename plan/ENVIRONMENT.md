# Pinned Environment

## Planning pins

- CMake 3.30.5
- Ninja 1.12.1
- C++20
- JUCE 9.0.4, commit `3fb5cbc0340758596dac39f7c8de47d562b4896e`
- Catch2 v3.16.1, commit `08092139210881f01a34e60c5f2fd6afa0ea4024`
- pluginval v1.0.4, commit `ed19c2c16b57a6d94db391bea3ef4a80b769d5bf`

## Verified sandbox binaries

- GCC/G++ 12.2.0 (Debian 12.2.0-14+deb12u1)
- CMake 3.30.5 and CTest 3.30.5 activated from `/tmp/cmake-3.30.5`
- Ninja 1.12.1 activated from `/tmp/ninja-1.12.1/ninja`
- CMake source override for red build: `FETCHCONTENT_SOURCE_DIR_CATCH2=/tmp/deps/Catch2`

## Literal red-test commands

```bash
FETCHCONTENT_SOURCE_DIR_CATCH2=/tmp/deps/Catch2 \
  /tmp/cmake-3.30.5/bin/cmake -S . -B /tmp/faultcounter-red -G Ninja \
  -DCMAKE_MAKE_PROGRAM=/tmp/ninja-1.12.1/ninja \
  -DCMAKE_BUILD_TYPE=Debug -DFAULTCOUNTER_BUILD_TESTS=ON
/tmp/cmake-3.30.5/bin/cmake --build /tmp/faultcounter-red --parallel 4
/tmp/faultcounter-red/FaultCounterTests --reporter compact
sha256sum -c tests/FROZEN_MANIFEST.sha256
```

The local source dependencies were cloned from the pinned repository tags and verified by exact commit hash. Public CI will fetch the same commits; the source-tree overrides above are only a sandbox reachability workaround and are not recorded as version substitutions.
