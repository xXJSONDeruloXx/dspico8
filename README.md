# dspico8

A Nintendo DS-focused, **clean native PICO-8 runtime experiment**.

The repo now has two tracks:

- **native runtime**: a new clean runtime in `native/` with a DS-first 8bpp framebuffer design
- **baseline runtime**: the earlier FAKE-08-derived bootstrap kept for comparison in `platform/nds` and the legacy source tree

## Current status

### Native runtime

`make nds` now builds the **new native runtime** from:

- `native/`
- `third_party/lua/`
- `platform/nds-native/`

This runtime is intentionally early and currently focuses on:

- `.p8` text carts
- core frame loop: `_init`, `_update` / `_update60`, `_draw`
- a practical graphics subset: `cls`, `color`, `pset`, `pget`, `line`, `rect`, `rectfill`, `spr`, `map`, `sget`, `sset`, `mget`, `mset`, `fget`, `fset`, `camera`, `clip`
- input: `btn`, `btnp`
- helpers: `rnd`, `time`, `flip`, plus a few Lua helper shims

### Baseline runtime

`make nds-baseline` still builds the FAKE-08-derived DS port for comparison and benchmarking.

### Rust foundations

A new Rust-first track now lives in `native-rs/`.

Current Rust work focuses on:

- safe `.p8` cart parsing
- safe `.p8.png` cart parsing
- safe framebuffer / sprite / map core primitives
- a minimal Lua bridge for desktop benchmarking
- desktop-subset API coverage including `btn`, `btnp`, `fget`, `fset`, and `rnd`
- probes for investigating user-supplied official `pico8.dat`

This is the preferred landing zone for new clean-runtime foundation work while the C++ native runtime remains the current DS reference implementation.

## Why this split exists

You asked for a native implementation that is **not FAKE-08**.

So the project now keeps:

- the old baseline as a benchmark / reference target
- a new clean runtime as the main target

This lets us measure speed and behavior against the previous implementation while moving the shipping artifact onto a non-FAKE-08 core.

## Docs

- `docs/research.md`
- `docs/architecture.md`
- `docs/roadmap.md`
- `docs/benchmarks.md`
- `docs/framehashes.md`
- `docs/rust-port.md`
- `docs/official-pico8-investigation.md`

## Build

Clone with submodules:

```bash
git clone --recurse-submodules https://github.com/xXJSONDeruloXx/dspico8.git
cd dspico8
```

Run regression tests for the baseline runtime:

```bash
make tests
```

Run Rust foundation tests:

```bash
make rust-tests
```

Run benchmark comparisons:

```bash
make benchmarks
```

Run framebuffer-hash comparisons between the current C++ runtime and Rust runtime on shared carts:

```bash
make framehashes
```

Build the new native Nintendo DS artifact:

```bash
make nds
```

Output:

```text
platform/nds-native/DSPICO8.nds
```

Build the baseline FAKE-08-derived DS artifact:

```bash
make nds-baseline
```

Output:

```text
platform/nds/DSPICO8.nds
```

### Docker build

```bash
./scripts/build-nds-docker.sh
```

This expects a running Docker daemon and uses the `devkitpro/devkitarm` image.

### Rust probes

Inspect carts with the Rust loader:

```bash
cargo run --manifest-path native-rs/Cargo.toml --bin cart_probe -- test/carts/cartparsetest.p8.png
```

Run the Rust prototype benchmark runtime:

```bash
cargo run --release --manifest-path native-rs/Cargo.toml --bin benchmark_native_rs -- bench/carts/fillrate.p8
```

Inspect a user-supplied official `pico8.dat`:

```bash
cargo run --manifest-path native-rs/Cargo.toml --bin pico8_dat_probe -- /path/to/pico8.dat
```

## Running on hardware

Current native runtime behavior:

- looks for `.p8` carts in `/p8carts`
- if none are found, falls back to a built-in benchmark cart
- game renders on the main screen
- runtime stats print on the sub screen

Controls:

- D-pad = directions
- `B` = PICO-8 `O`
- `A` = PICO-8 `X`
- `START` = `btn(6)`
- hold `L + R` to quit

## Benchmarks

Current desktop microbenchmarks compare three runtimes:

- FAKE-08-derived baseline
- current clean native C++ runtime
- Rust prototype runtime

On the current included carts, the Rust prototype is the fastest of the three on desktop, but it is still a desktop prototype and is not yet wired into the DS build.

For two shared subset carts, the Rust and C++ runtimes currently produce identical framebuffer hashes after the same number of steps.

See `docs/benchmarks.md` and `docs/framehashes.md` for the raw numbers.

## Provenance

This repo still contains the earlier FAKE-08-derived baseline for comparison purposes, but the main runtime direction now centers on the clean implementation under `native/`.

Reference inspirations are documented in `docs/research.md`.

See `LICENSE.MD` and third-party library directories for license details.
