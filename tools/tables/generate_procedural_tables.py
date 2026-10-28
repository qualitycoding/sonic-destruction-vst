#!/usr/bin/env python3
"""Generate the frozen clean-room 252x256 procedural transfer table."""
from __future__ import annotations
import hashlib, json
from pathlib import Path

TABLE_COUNT = 252
ROW_LENGTH = 256
VARIANTS = 28

def clamp_s(v: int) -> int:
    return max(-128, min(127, v))

def sign(v: int) -> int:
    return -1 if v < 0 else (1 if v > 0 else 0)

def row_hash(row: bytes) -> str:
    return hashlib.sha256(row).hexdigest()

def saturation(k: int, c: int) -> int:
    x = c - 128
    p = k + 1
    return clamp_s(x * p)

def signed_power(k: int, c: int) -> int:
    x = c - 128
    e = 1 + (k % 4)
    a = 1 + (k // 4)
    value = (a * abs(x) ** e + 128 ** (e - 1) // 2) // (128 ** (e - 1))
    return sign(x) * min(127, value)

def chebyshev(k: int, c: int) -> int:
    x = c - 128
    n = 1 + (k % 7)
    a = 1 + (k // 7)
    t0 = 32768
    t1 = x * 256
    prev2, prev1 = t0, t1
    for _ in range(2, n + 1):
        current = (2 * t1 * prev1) // 32768 - prev2
        prev2, prev1 = prev1, current
    tn = prev1
    return clamp_s((a * tn) // 256)

def triangle_fold(k: int, c: int) -> int:
    x = c - 128
    p = k + 1
    z = (x * p + 128) % 512
    t = z if z < 256 else 511 - z
    return t - 128

def modulo_wrap(k: int, c: int) -> int:
    x = c - 128
    p = k + 1
    return (x * p + 128) % 256 - 128

def xor_mask(k: int, c: int) -> int:
    return c ^ ((73 * (k + 1)) % 256)

def rotl8(v: int, r: int) -> int:
    return ((v << r) | (v >> (8 - r))) & 0xFF

def rotr8(v: int, r: int) -> int:
    return ((v >> r) | (v << (8 - r))) & 0xFF

def bitrev8(v: int) -> int:
    return int(f'{v:08b}'[::-1], 2)

def gray(v: int) -> int:
    return v ^ (v >> 1)

def rotate_reversal(k: int, c: int) -> int:
    r = 1 + (k % 7)
    g = k // 7
    if g == 0:
        base = rotl8(c, r)
    elif g == 1:
        base = rotr8(c, r)
    elif g == 2:
        base = rotl8(bitrev8(c), r)
    else:
        base = rotl8(gray(c), r)
    return base ^ ((17 * k) % 256)

def signed_multiplication(k: int, c: int) -> int:
    x = c - 128
    p = k + 1
    return sign(x) * min(127, (x * x * p) // 16384)

def asymmetric_threshold(k: int, c: int) -> int:
    x = c - 128
    t = 8 + 4 * (k % 7)
    a = 1 + (k // 7)
    if x >= 0:
        return min(127, 4 * a * max(0, x - t))
    return max(-128, -2 * a * max(0, -x - 2 * t))

FAMILIES = [
    ('saturation', saturation),
    ('signed-power', signed_power),
    ('chebyshev', chebyshev),
    ('triangle-fold', triangle_fold),
    ('modulo-wrap', modulo_wrap),
    ('xor-mask', xor_mask),
    ('rotate-reversal', rotate_reversal),
    ('signed-multiplication', signed_multiplication),
    ('asymmetric-threshold', asymmetric_threshold),
]

def severity(y: list[int]) -> int:
    diffs = [y[i + 1] - y[i] for i in range(255)]
    changes = sum(1 for a, b in zip(diffs, diffs[1:]) if a * b < 0)
    return sum(abs(v - (i - 128)) for i, v in enumerate(y)) + 256 * changes

def main() -> None:
    rows: list[bytes] = []
    manifest_rows: list[dict[str, object]] = []
    for family_index, (family, function) in enumerate(FAMILIES):
        candidates: list[tuple[int, int, int, bytes]] = []
        for k in range(VARIANTS):
            y = [function(k, c) for c in range(ROW_LENGTH)]
            if family not in ('xor-mask', 'rotate-reversal'):
                row = bytes(clamp_s(v) + 128 for v in y)
            else:
                row = bytes(y)
            candidates.append((severity(y), k, severity(y), row))
        candidates.sort(key=lambda x: (x[0], x[1]))
        unique = {candidate[3] for candidate in candidates}
        if len(unique) != VARIANTS:
            raise SystemExit(f'{family}: expected 28 unique rows, got {len(unique)}')
        for sorted_index, (sev, k, _, row) in enumerate(candidates):
            rows.append(row)
            manifest_rows.append({
                'table': family_index * VARIANTS + sorted_index,
                'family': family,
                'variant': k,
                'severity': sev,
                'sha256': row_hash(row),
            })
    if len(rows) != TABLE_COUNT:
        raise SystemExit(f'Expected {TABLE_COUNT} rows, got {len(rows)}')
    asset = b''.join(rows)
    if len(asset) != TABLE_COUNT * ROW_LENGTH:
        raise SystemExit('Unexpected asset length')
    out = Path('assets/tables')
    out.mkdir(parents=True, exist_ok=True)
    (out / 'procedural.bin').write_bytes(asset)
    (out / 'procedural-manifest.json').write_text(json.dumps({
        'schema': 1,
        'tables': TABLE_COUNT,
        'rowLength': ROW_LENGTH,
        'assetSha256': hashlib.sha256(asset).hexdigest(),
        'rows': manifest_rows,
    }, indent=2) + '\n', encoding='utf-8')
    print(hashlib.sha256(asset).hexdigest())

if __name__ == '__main__':
    main()
