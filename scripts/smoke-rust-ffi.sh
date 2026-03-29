#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="$ROOT_DIR/.ffi-smoke"
mkdir -p "$OUT_DIR"

cargo build --release --manifest-path "$ROOT_DIR/native-rs/Cargo.toml"

c++ -O3 -std=c++17 \
  -I"$ROOT_DIR/native-rs/include" \
  "$ROOT_DIR/bench/framehash_native_rs_ffi.cpp" \
  "$ROOT_DIR/native-rs/target/release/libdsp_native_rs.a" \
  -ldl -lm -lpthread \
  -o "$OUT_DIR/framehash_native_rs_ffi"

c++ -O3 -std=c++17 -DDSP_NATIVE_USE_RUST_WRAPPER \
  -I"$ROOT_DIR/native/include" \
  -I"$ROOT_DIR/native-rs/include" \
  "$ROOT_DIR/bench/framehash_native.cpp" \
  "$ROOT_DIR/native/src/dsp_native_runtime_rs.cpp" \
  "$ROOT_DIR/native-rs/target/release/libdsp_native_rs.a" \
  -ldl -lm -lpthread \
  -o "$OUT_DIR/framehash_native_rs_cpp"

c++ -O3 -std=c++17 \
  -I"$ROOT_DIR/native/include" \
  -I"$ROOT_DIR/native-rs/include" \
  "$ROOT_DIR/bench/smoke_runtime_rs_builtin.cpp" \
  "$ROOT_DIR/native/src/dsp_native_runtime_rs.cpp" \
  "$ROOT_DIR/native-rs/target/release/libdsp_native_rs.a" \
  -ldl -lm -lpthread \
  -o "$OUT_DIR/smoke_runtime_rs_builtin"

"$OUT_DIR/framehash_native_rs_ffi" "$ROOT_DIR/bench/carts/fillrate.p8" 120
"$OUT_DIR/framehash_native_rs_ffi" "$ROOT_DIR/bench/carts/sprite_stress.p8" 120
"$OUT_DIR/framehash_native_rs_ffi" "$ROOT_DIR/bench/carts/api_mix_det.p8" 120

"$OUT_DIR/framehash_native_rs_cpp" "$ROOT_DIR/bench/carts/fillrate.p8" 120
"$OUT_DIR/framehash_native_rs_cpp" "$ROOT_DIR/bench/carts/sprite_stress.p8" 120
"$OUT_DIR/framehash_native_rs_cpp" "$ROOT_DIR/bench/carts/api_mix_det.p8" 120

"$OUT_DIR/smoke_runtime_rs_builtin"
