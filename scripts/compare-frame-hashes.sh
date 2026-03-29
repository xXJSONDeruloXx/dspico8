#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
HASH_DIR="$ROOT_DIR/.framehash"
NATIVE_DIR="$HASH_DIR/native"
mkdir -p "$NATIVE_DIR"

cc -std=c99 -DMAKE_LIB -I"$ROOT_DIR/third_party/lua" -c "$ROOT_DIR/third_party/lua/onelua.c" -o "$NATIVE_DIR/onelua.o"
c++ -O3 -std=c++17 \
  -I"$ROOT_DIR/native/include" \
  -I"$ROOT_DIR/third_party/lua" \
  "$ROOT_DIR/bench/framehash_native.cpp" \
  "$ROOT_DIR/native/src/dsp_native_cart.cpp" \
  "$ROOT_DIR/native/src/dsp_native_runtime.cpp" \
  "$NATIVE_DIR/onelua.o" \
  -lm -ldl \
  -o "$NATIVE_DIR/framehash_native"

CARTS=(
  "$ROOT_DIR/bench/carts/fillrate.p8"
  "$ROOT_DIR/bench/carts/sprite_stress.p8"
  "$ROOT_DIR/bench/carts/api_mix_det.p8"
)

RESULTS_FILE="$HASH_DIR/results.txt"
: > "$RESULTS_FILE"
for cart in "${CARTS[@]}"; do
  "$NATIVE_DIR/framehash_native" "$cart" 120 | tee -a "$RESULTS_FILE"
  cargo run --release --manifest-path "$ROOT_DIR/native-rs/Cargo.toml" --bin frame_hash_native_rs -- "$cart" 120 | tee -a "$RESULTS_FILE"
done

python3 - <<'PY' "$RESULTS_FILE" "$ROOT_DIR/docs/framehashes.md"
import sys
from pathlib import Path

results_path = Path(sys.argv[1])
out_path = Path(sys.argv[2])
entries = []
for line in results_path.read_text().splitlines():
    parts = {}
    for token in line.strip().split():
        if '=' in token:
            k, v = token.split('=', 1)
            parts[k] = v
    if parts:
        entries.append(parts)

by_cart = {}
for e in entries:
    by_cart.setdefault(e['cart'], {})[e['runtime']] = e

lines = [
    '# Framebuffer hash comparisons',
    '',
    'Framebuffer FNV-1a hashes comparing the current clean C++ runtime against the Rust runtime on shared subset carts.',
    '',
    '| Cart | Native C++ hash | Native Rust hash | Match |',
    '| --- | --- | --- | --- |',
]

for cart, runtimes in sorted(by_cart.items()):
    native = runtimes.get('native-hash')
    rust = runtimes.get('native-rs-hash')
    if not native or not rust:
        continue
    match = 'yes' if native['fnv64'] == rust['fnv64'] else 'no'
    lines.append(
        f"| `{Path(cart).name}` | `{native['fnv64']}` | `{rust['fnv64']}` | {match} |"
    )

lines += [
    '',
    '## Raw output',
    '',
    '```text',
    results_path.read_text().rstrip(),
    '```',
]

out_path.write_text('\n'.join(lines) + '\n')
print('\n'.join(lines))
PY
