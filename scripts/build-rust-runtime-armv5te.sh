#!/usr/bin/env bash
set -euo pipefail

if [ -d /opt/homebrew/opt/rustup/bin ]; then
  export PATH="/opt/homebrew/opt/rustup/bin:$HOME/.cargo/bin:$PATH"
fi

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
ARM_GCC="/opt/devkitpro/devkitARM/bin/arm-none-eabi-gcc"
ARM_AR="/opt/devkitpro/devkitARM/bin/arm-none-eabi-ar"
ARM_RANLIB="/opt/devkitpro/devkitARM/bin/arm-none-eabi-ranlib"

command -v rustup >/dev/null 2>&1 || { echo "rustup is required for build-rust-runtime-armv5te"; exit 1; }
command -v cargo >/dev/null 2>&1 || { echo "cargo is required for build-rust-runtime-armv5te"; exit 1; }
[ -x "$ARM_GCC" ] || { echo "missing $ARM_GCC"; exit 1; }
[ -x "$ARM_AR" ] || { echo "missing $ARM_AR"; exit 1; }
[ -x "$ARM_RANLIB" ] || { echo "missing $ARM_RANLIB"; exit 1; }

rustup toolchain install nightly --component rust-src >/dev/null

env \
  RUSTFLAGS='-Zunstable-options -Cpanic=immediate-abort' \
  CC_armv5te_none_eabi="$ARM_GCC" \
  AR_armv5te_none_eabi="$ARM_AR" \
  RANLIB_armv5te_none_eabi="$ARM_RANLIB" \
  CARGO_TARGET_ARMV5TE_NONE_EABI_LINKER="$ARM_GCC" \
  cargo +nightly build \
    -Z build-std=core,alloc,compiler_builtins \
    --target armv5te-none-eabi \
    --release \
    --manifest-path "$ROOT_DIR/native-rs/Cargo.toml" \
    --no-default-features \
    --lib
