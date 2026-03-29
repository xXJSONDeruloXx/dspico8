# dspico8

A Nintendo DS-focused, native PICO-8-compatible runtime experiment.

`dspico8` bootstraps from the proven FAKE-08 core, then layers a DS-native host, build pipeline, and DS-specific performance work on top.

## What is in this repo

- a working Nintendo DS ARM7/ARM9 build
- the FAKE-08-compatible runtime core and desktop regression tests
- DS-specific host code built with `libnds` + `maxmod`
- research / planning / roadmap docs in `docs/`
- GitHub Actions for tests, `.nds` builds, and tagged prereleases

## Docs

- `docs/research.md`
- `docs/architecture.md`
- `docs/roadmap.md`

## Build

Clone with submodules:

```bash
git clone --recurse-submodules https://github.com/xXJSONDeruloXx/dspico8.git
cd dspico8
```

Run desktop regression tests:

```bash
make tests
```

Build the Nintendo DS artifact with a local devkitPro install:

```bash
make nds
```

Output:

```text
platform/nds/DSPICO8.nds
```

### Docker build

A convenience script is included for Docker-based builds:

```bash
./scripts/build-nds-docker.sh
```

This expects a running Docker daemon and uses the `devkitpro/devkitarm` image.

## Running on hardware

- copy carts into `/p8carts`
- launch `DSPICO8.nds`
- D-pad = directions
- `B` = PICO-8 `O`
- `A` = PICO-8 `X`
- `START` = pause/menu
- `SELECT` = stretch/screen mode
- hold `L + R` to quit

## Current implementation direction

Short version:

- use the FAKE-08 native core as the compatibility bootstrap
- keep the DS host thin and explicit
- optimize DS-specific hot paths first

The first DS-focused optimization already in this repo is a faster framebuffer upload path that expands packed PICO-8 pixels with a LUT instead of per-pixel nibble extraction.

## Provenance

This repo is derived from and inspired by:

- `xXJSONDeruloXx/ds-fake-08` (`feat/nds-support`)
- `jtothebell/fake-08`
- `jtothebell/z8lua`
- broader PICO-8 implementation references documented in `docs/research.md`

See `LICENSE.MD` and third-party library directories for license details.
