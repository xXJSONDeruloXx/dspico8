# Rust port plan

## Why pivot now

The current clean native runtime in `native/` proves that a non-FAKE-08 direction is viable, but it is **not** yet at feature parity with the FAKE-08-derived baseline.

Because the long-term preference is now:

- memory safety
- maintainability
- avoiding more major C/C++ investment

new feature work should start shifting toward a Rust implementation.

## Goals

1. keep the runtime core as safe Rust wherever practical
2. isolate unsafe code to small FFI / hardware boundaries
3. preserve current performance wins over the FAKE-08 baseline
4. use the current C++ clean runtime only as a reference during migration

## Proposed split

### `native-rs/`

Rust workspace / crate(s) for the next-generation native runtime.

Initial responsibilities:

- `.p8` parsing
- `.p8.png` parsing
- PICO-8 memory buffers in safe Rust
- framebuffer + draw primitives in safe Rust
- optional probes for user-supplied `pico8.dat`

Later responsibilities:

- Lua bridge layer
- DS host integration
- performance tuning on ARM9
- compatibility expansion

## Unsafe boundary rules

Unsafe code should be limited to:

- Lua C API bindings
- `libnds` / VRAM / DMA / interrupts
- low-level startup / linkage glue

Everything else should prefer:

- slices / arrays
- explicit bounds checks
- allocation-free hot loops
- plain data structs

## Performance gate

Rust is acceptable only if it stays competitive.

Current rule of thumb:

- target within ~5–10% of the current clean C++ runtime on shared microbenchmarks
- if Rust is slower in a hotspot, optimize that hotspot before widening scope
- use tiny localized `unsafe` only when measurement justifies it

## Migration sequence

### Phase 1 — safe core foundations

- Rust cart parsing for `.p8`
- Rust cart parsing for `.p8.png`
- Rust framebuffer / spritesheet / map / flags storage
- Rust drawing primitives
- Rust test coverage for these pieces

### Phase 2 — execution bridge

- minimal Lua bridge in Rust
- keep unsafe contained to the binding layer
- reuse the current API subset as the initial parity target

### Phase 3 — host validation

- run desktop tests for Rust modules
- add desktop microbenchmarks for Rust runtime pieces
- compare against the current C++ clean runtime and FAKE-08 baseline

### Phase 4 — DS integration

- compile Rust runtime as a static library for the DS build
- keep ARM7 / ARM9 shell minimal at first
- prove frame loop, cart loading, and rendering on emulator / hardware

### Phase 5 — expansion

- `.p8.png` cart UX on DS
- optional user-supplied `/pico8/pico8.dat` support where technically useful
- audio, save data, and broader API coverage

## Practical rule for the repo

For now:

- `native/` remains the working C++ reference runtime
- `native-rs/` becomes the preferred destination for new clean-runtime work
- avoid large new C++ compatibility features unless they are strictly needed to unblock the Rust path
