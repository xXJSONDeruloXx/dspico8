#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"

exec docker run --rm \
  -v "$ROOT_DIR:/build_dir" \
  devkitpro/devkitarm \
  /bin/bash -lc '
    source /etc/profile.d/devkit-env.sh
    apt-get update
    apt-get install -y p7zip-full zip libc6-dev g++
    cd /build_dir
    make nds
  '
