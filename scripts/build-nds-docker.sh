#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"

exec docker run --rm \
  -v "$ROOT_DIR:/build_dir" \
  devkitpro/devkitarm \
  /bin/bash -lc '
    source /etc/profile.d/devkit-env.sh
    apt-get update
    apt-get install -y p7zip-full zip libc6-dev g++ curl ca-certificates
    curl https://sh.rustup.rs -sSf | sh -s -- -y --profile minimal --default-toolchain nightly
    . "$HOME/.cargo/env"
    rustup component add rust-src
    cd /build_dir
    make clean-nds || true
    make nds
    make nds-baseline
    make nds-rust-core-smoke
    mkdir -p artifacts
    cp platform/nds-native/DSPICO8.nds artifacts/DSPICO8.nds
    cp platform/nds/DSPICO8.nds artifacts/DSPICO8-FAKE08-BASELINE.nds
    cp platform/nds-rust-core/DSPICO8-RUST-CORE.nds artifacts/DSPICO8-RUST-CORE.nds
  '
