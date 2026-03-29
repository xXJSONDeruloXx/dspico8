#!/usr/bin/env bash
set -euo pipefail

if [ -d /opt/homebrew/opt/rustup/bin ]; then
  export PATH="/opt/homebrew/opt/rustup/bin:$HOME/.cargo/bin:$PATH"
fi

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"

command -v rustup >/dev/null 2>&1 || { echo "rustup is required for rust-core-arm-check"; exit 1; }
command -v cargo >/dev/null 2>&1 || { echo "cargo is required for rust-core-arm-check"; exit 1; }

rustup toolchain install nightly --component rust-src >/dev/null
cargo +nightly build -Z build-std=core --target armv5te-none-eabi --manifest-path "$ROOT_DIR/native-rs-core/Cargo.toml"
