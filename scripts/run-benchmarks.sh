#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BENCH_DIR="$ROOT_DIR/.bench"
NATIVE_DIR="$BENCH_DIR/native"
FAKE_DIR="$BENCH_DIR/fake08"
mkdir -p "$NATIVE_DIR" "$FAKE_DIR"

pushd "$ROOT_DIR" >/dev/null
make tests >/dev/null
popd >/dev/null

cc -std=c99 -DMAKE_LIB -I"$ROOT_DIR/third_party/lua" -c "$ROOT_DIR/third_party/lua/onelua.c" -o "$NATIVE_DIR/onelua.o"
c++ -O3 -std=c++17 \
  -I"$ROOT_DIR/native/include" \
  -I"$ROOT_DIR/third_party/lua" \
  "$ROOT_DIR/bench/benchmark_native.cpp" \
  "$ROOT_DIR/native/src/dsp_native_cart.cpp" \
  "$ROOT_DIR/native/src/dsp_native_runtime.cpp" \
  "$NATIVE_DIR/onelua.o" \
  -lm -ldl \
  -o "$NATIVE_DIR/benchmark_native"

FAKE08_OBJECTS=(
  Audio.o cart.o emojiconversion.o filehelpers.o filter.o fontdata.o graphics.o Input.o
  mathhelpers.o nibblehelpers.o picoluaapi.o printHelper.o stringToDataHelpers.o synth.o vm.o
  lodepng.o stubhost.o nooplogger.o eris.o lapi.o lauxlib.o lbaselib.o lbitlib.o lcode.o
  lcorolib.o lctype.o ldblib.o ldebug.o ldo.o ldump.o lfunc.o lgc.o linit.o liolib.o
  llex.o lmem.o loadlib.o lobject.o lopcodes.o loslib.o lparser.o lpico8lib.o lstate.o
  lstring.o lstrlib.o ltable.o ltablib.o ltests.o ltm.o lundump.o lvm.o lzio.o ConvertUTF.o miniz.o
)

FAKE08_LINK_ARGS=()
for obj in "${FAKE08_OBJECTS[@]}"; do
  FAKE08_LINK_ARGS+=("$ROOT_DIR/test/build/$obj")
done

c++ -O3 -std=c++17 \
  -I"$ROOT_DIR/source" \
  -I"$ROOT_DIR/libs/z8lua" \
  -I"$ROOT_DIR/libs/lodepng" \
  -I"$ROOT_DIR/libs/simpleini" \
  -I"$ROOT_DIR/libs/miniz" \
  -I"$ROOT_DIR/test" \
  "$ROOT_DIR/bench/benchmark_fake08.cpp" \
  "${FAKE08_LINK_ARGS[@]}" \
  -o "$FAKE_DIR/benchmark_fake08"

CARTS=(
  "$ROOT_DIR/bench/carts/fillrate.p8"
  "$ROOT_DIR/bench/carts/sprite_stress.p8"
  "$ROOT_DIR/bench/carts/api_mix.p8"
)

RESULTS_FILE="$BENCH_DIR/results.txt"
: > "$RESULTS_FILE"
for cart in "${CARTS[@]}"; do
  "$FAKE_DIR/benchmark_fake08" "$cart" 120 600 | tee -a "$RESULTS_FILE"
  "$NATIVE_DIR/benchmark_native" "$cart" 120 600 | tee -a "$RESULTS_FILE"
  cargo run --release --manifest-path "$ROOT_DIR/native-rs/Cargo.toml" --bin benchmark_native_rs -- "$cart" 120 600 | tee -a "$RESULTS_FILE"
done

python3 - <<'PY' "$RESULTS_FILE" "$ROOT_DIR/docs/benchmarks.md"
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
    '# Benchmarks',
    '',
    'Desktop microbenchmarks comparing the FAKE-08-derived baseline, the current clean C++ runtime, and the new Rust prototype runtime.',
    '',
    '| Cart | Fake08 us/frame | Native C++ us/frame | Native Rust us/frame | Fastest runtime | Rust vs C++ |',
    '| --- | ---: | ---: | ---: | --- | ---: |',
]

for cart, runtimes in sorted(by_cart.items()):
    fake = runtimes.get('fake08')
    native = runtimes.get('native')
    native_rs = runtimes.get('native-rs')
    if not fake or not native or not native_rs:
        continue
    fake_us = float(fake['us_per_frame'])
    native_us = float(native['us_per_frame'])
    native_rs_us = float(native_rs['us_per_frame'])

    fastest_runtime, fastest_us = min(
        [('fake08', fake_us), ('native', native_us), ('native-rs', native_rs_us)],
        key=lambda item: item[1],
    )
    rust_vs_cpp = native_us / native_rs_us if native_rs_us else 0.0
    lines.append(
        f'| `{Path(cart).name}` | {fake_us:.2f} | {native_us:.2f} | {native_rs_us:.2f} | {fastest_runtime} | {rust_vs_cpp:.2f}x |'
    )

lines += [
    '',
    '## Emulator smoke notes',
    '',
    '- Installed macOS emulators: `melonDS`, `DeSmuME`',
    '- `melonDS` successfully opens both `.nds` files for quick smoke testing',
    '- current observation:',
    '  - FAKE-08-derived baseline reaches its startup text in `melonDS` (`FAT init failed.` with the current emulator filesystem config)',
    '  - the new native runtime currently boots to a black screen in `melonDS`, so desktop benchmark numbers remain the current source of truth for performance comparisons while DS-emulator bring-up continues',
    '- Rust status:',
    '  - the Rust runtime is currently a desktop prototype with a minimal Lua bridge and safe Rust core primitives',
    '  - it is not yet wired into the DS build, but it is now benchmarkable on desktop against the existing C++ runtime',
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
