#!/usr/bin/env python3
"""Generate frozen expected PCM for the independent engine reference."""
from __future__ import annotations
import hashlib, json, math, struct, subprocess
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ASSET = ROOT / 'assets/tables/procedural.bin'
OUT = ROOT / 'tests/golden'
LENGTH = 512
SAMPLE_RATE = 48000
REQUESTED_RATE = 9000
BITS = 3
TABLE = 42
POSITION = 'post'  # must match 'position' in engine-golden-manifest.json; D-007 Post = reduce after table

def bits(code: int, depth: int) -> int:
    levels = 2 ** depth - 1
    q = math.floor((code / 255) * levels + 0.5)
    return math.floor((q / levels) * 255 + 0.5)

def zoh(samples: list[float]) -> list[float]:
    phase = 1.0
    held = 0.0
    out = []
    for sample in samples:
        phase += REQUESTED_RATE / SAMPLE_RATE
        if phase >= 1.0:
            held = sample
            phase -= math.floor(phase)
        out.append(held)
    return out

def reference(samples: list[float], tables: bytes) -> list[float]:
    held = zoh(samples)
    coefficient = 1.0 - (2.0 * math.pi * 5.0 / SAMPLE_RATE)
    previous_input = 0.0
    previous_output = 0.0
    output = []
    for index, sample in enumerate(held):
        code = max(0, min(255, math.floor((max(-1.0, min(1.0, sample)) + 1.0) * 0.5 * 255.0 + 0.5)))
        mapped = tables[TABLE * 256 + code]
        if POSITION == 'post':
            mapped = bits(mapped, BITS)
        elif POSITION != 'pre':
            raise ValueError(f'unknown position {POSITION!r}')
        normalized = 2.0 * (mapped / 255.0) - 1.0
        value = normalized - previous_input + coefficient * previous_output
        previous_input = normalized
        previous_output = value
        output.append(value)
    return output

def inputs() -> dict[str, list[float]]:
    impulse = [0.0] * LENGTH
    impulse[0] = 1.0
    ramp = [-1.0 + 2.0 * i / (LENGTH - 1) for i in range(LENGTH)]
    sine = [0.5 * math.sin(2.0 * math.pi * 997.0 * i / SAMPLE_RATE) for i in range(LENGTH)]
    state = 0x47C0
    noise = []
    for _ in range(LENGTH):
        state = (state * 1664525 + 1013904223) & 0xFFFFFFFF
        noise.append(state / 0x80000000 - 1.0)
    return {
        'impulse': impulse,
        'ramp': ramp,
        'sine': sine,
        'noise-seed-0x47c0': noise,
        'silence': [0.0] * LENGTH,
    }

def environment_lock() -> str:
    """SHA-256 of the pinned environment file (T-023 environment-lock hash)."""
    return hashlib.sha256((ROOT / 'plan/ENVIRONMENT.md').read_bytes()).hexdigest()


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    tables = ASSET.read_bytes()
    manifest = {
        'schema': 2,
        'generator': 'tools/tests/generate_engine_golden.py',
        'commit': subprocess.run(
            ['git', 'rev-parse', 'HEAD'], cwd=ROOT, capture_output=True, text=True, check=True
        ).stdout.strip(),
        'environmentLock': environment_lock(),
        'generatedUtc': datetime.now(timezone.utc).strftime('%Y-%m-%dT%H:%M:%SZ'),
        'seed': 0x47C0,  # LCG seed of the deterministic noise fixture
        'sampleRate': SAMPLE_RATE,
        'blockSchedule': [LENGTH],  # EngineGoldenTest processes one block of `length`
        'parameterState': {
            'sampleRate': REQUESTED_RATE,
            'bits': BITS,
            'table': TABLE,
            'position': POSITION,
        },
        # Legacy top-level keys retained for human diff continuity.
        'requestedRate': REQUESTED_RATE,
        'bits': BITS,
        'table': TABLE,
        'position': POSITION,
        'length': LENGTH,
        'fixtures': {},
    }
    for name, samples in inputs().items():
        expected = reference(samples, tables)
        data = struct.pack('<' + 'f' * len(expected), *expected)
        path = OUT / f'expected-{name}.f32'
        path.write_bytes(data)
        manifest['fixtures'][name] = {
            'path': str(path.relative_to(ROOT)),
            'sha256': hashlib.sha256(data).hexdigest(),
            'bytes': len(data),
        }
    (OUT / 'engine-golden-manifest.json').write_text(json.dumps(manifest, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(manifest['fixtures'], indent=2))

if __name__ == '__main__':
    main()
