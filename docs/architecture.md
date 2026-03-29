# Architecture plan

## High-level shape

`dspico8` is split into four major layers:

1. **Runtime core**
   - cartridge parsing (`.p8`, `.p8.png`)
   - PICO-8 memory model
   - drawing API
   - input API
   - audio API
   - Lua VM integration

2. **Nintendo DS platform host**
   - ARM9 video, filesystem, menu/input glue
   - ARM7 system services, touch/audio support
   - palette upload, framebuffer transfer, scaling, timing

3. **Tooling + tests**
   - native regression tests for the runtime core
   - deterministic build scripts
   - CI and release workflows

4. **Docs + roadmap**
   - research notes
   - compatibility and perf notes
   - release/build instructions

## Runtime core

The bootstrap runtime core comes from the FAKE-08 architecture:

- `Vm`
- `Graphics`
- `Audio`
- `Input`
- `Cart`
- `PicoRam`
- `z8lua`

Why keep this initially:

- it already runs a broad set of carts,
- it already has tests,
- and it lets DS work focus on the real bottleneck: host-side performance and renderer hot paths.

## Nintendo DS host split

### ARM9 responsibilities

- initialize FAT / SD filesystem
- own UI flow and cart loading
- poll buttons and touch state
- run the runtime core
- push palette + framebuffer data to VRAM
- handle screen scaling / screen placement

### ARM7 responsibilities

- startup services
- touch server
- sound services
- maxmod installation and low-level DS helpers

This keeps the design close to normal DS homebrew practice and makes release builds easier to reason about.

## Rendering model

### Source format

The PICO-8 framebuffer is naturally represented as:

- 128x128 pixels
- 16 colors
- packed 4bpp (two pixels per byte)

### DS display target

The DS host uses an 8bpp affine bitmap background for the game image so we can:

- keep a 16-color palette,
- scale the 128x128 output using affine transforms,
- move the image between top/bottom screens,
- and preserve existing stretch modes.

### Hot path plan

The frame upload path should be treated as performance-sensitive code:

- precompute a `u16 expand[256]` table where each packed nibble byte becomes two 8-bit palette indices
- upload palette only when changed, or at least keep that code trivial
- convert the 8192-byte packed framebuffer into 16384 bytes of BG data using the LUT instead of repeated `getPixelNibble()` calls
- avoid per-pixel branching in the upload loop

Later options:

- dirty-row / dirty-rect tracking
- direct renderer paths that target a DS-friendly intermediate buffer
- specialized sprite/map routines for common cases

## Input model

Default mapping:

- D-pad -> PICO-8 directions
- B -> O button
- A -> X button
- Start -> pause/menu
- Select -> display/stretch mode switch
- Touch -> mouse emulation where carts use it

## Audio model

- keep streamed audio through `maxmod`
- preserve FAKE-08 audio semantics first
- optimize only after frame pacing is stable and correctness is verified

## Storage model

Expected filesystem layout on device:

- carts: `/p8carts`
- app data/logs: `/_nds/dspico8/` or compatible equivalent
- cart save data in a dedicated subdirectory

## Build model

### Local

Two supported paths:

1. installed devkitPro toolchain
2. Docker using `devkitpro/devkitarm`

### CI

GitHub Actions should:

- checkout submodules
- run desktop tests
- build the `.nds`
- upload artifacts
- optionally publish tagged prereleases

## Non-goals for v0

- rewriting the full VM from scratch before we have a shippable DS build
- trying to ship the official PICO-8 runtime
- over-designing a custom asset pipeline before compatibility and speed are good enough
