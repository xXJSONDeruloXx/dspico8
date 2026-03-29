# Research notes

## Goal

Build a practical, native-feeling Nintendo DS implementation for running PICO-8 cartridges, with a path to better DS performance than the current FAKE-08 DS port branch.

## Constraints

- The Nintendo DS is much slower than the 3DS / Switch / modern mobile targets FAKE-08 already serves.
- Shipping the official PICO-8 runtime is not an option for this repo.
- A real deliverable must exist: reproducible builds, a working `.nds`, and release packaging.

## Reference repo survey

### 1. `xXJSONDeruloXx/ds-fake-08` (`feat/nds-support`)

What it gives us:

- A **working native DS build path** using `libnds`, `maxmod`, FAT, and the standard ARM7/ARM9 split.
- A proven integration of the FAKE-08 core with a DS host implementation.
- Existing cart browsing, save-data directories, audio streaming, touch input, and affine background scaling modes.

What we observed:

- The branch builds successfully when cloned with submodules.
- The DS host is already native C/C++, not a browser or WASM wrapper.
- The current ARM9 frame upload path converts the packed 4bpp PICO-8 framebuffer to DS 8bpp background memory every frame in a CPU loop.
- The core still carries the full FAKE-08 software rendering / Lua execution cost, which is likely the dominant source of slowdown on DS hardware.

Takeaway:

- The fastest route to a real artifact is to **bootstrap from the FAKE-08 native core**, then add DS-specific fast paths.
- A total from-scratch clean-room VM is attractive long-term, but not the right first milestone if we want a usable `.nds` soon.

### 2. `jevonlipsey/pico-ios`

What matters here:

- Pocket8 uses an **official licensed PICO-8 runtime packaged as WASM** on iOS.
- The interesting engineering ideas are the **host/runtime boundary**, boot-time cartridge injection, and state serialization.

What does *not* transfer directly:

- DS does not have the headroom or legal/runtime assumptions that make an official WASM runtime practical here.
- The Nintendo DS target should stay focused on a native, open implementation.

Takeaway:

- Reuse the architectural lesson: keep the platform host thin and explicit.
- Borrow the idea of treating save states / cart persistence / host services as first-class systems.
- Do **not** base the DS build on a WASM player strategy.

### 3. `pico-8/awesome-PICO-8`

Useful signposts from the index:

- `fake-08`
- `zepto8`
- `tac08`
- `PicoLove`

Takeaway:

- There is already a strong ecosystem of partial and full reimplementations.
- For DS, the winning strategy is likely hybrid:
  - take the compatibility lessons from FAKE-08 / zepto8 / tac08,
  - but write DS-specific rendering, transfer, and profiling work as first-class code instead of treating DS like just another generic target.

### 4. `wh0am1-dev/pico8-api`

Value:

- A compact, easy-to-browse API reference for building a completeness checklist.

Takeaway:

- Use it as a documentation aid, not a source of truth.
- Track implemented API coverage and regressions against tests, not memory.

## Proposed strategy

### Phase 1: practical bootstrap

- Start from the working FAKE-08 DS-native branch structure.
- Narrow the repo around the DS deliverable.
- Preserve tests where practical.
- Ship a reproducible `.nds` quickly.

### Phase 2: DS-specific performance work

Prioritize the parts most likely to matter on Nintendo DS:

1. **Framebuffer upload path**
   - replace per-pixel nibble extraction with LUT-based packed expansion
   - reduce branching in the hot path
   - investigate DMA-friendly copies where possible

2. **Software renderer hotspots**
   - profile sprite blits, line/rect fill, text, and map rendering
   - specialize common cases for unmasked / unclipped operations

3. **Lua / VM overhead**
   - keep z8lua, but look for avoidable host crossings and redundant work
   - use tests to prevent compatibility regressions while optimizing

4. **Audio and frame pacing**
   - keep ARM7 services simple
   - make frame pacing stable before chasing micro-optimizations elsewhere

### Phase 3: compatibility + UX

- Build a documented compatibility matrix.
- Add a small cart smoke-test set.
- Improve save data handling, crash fallbacks, and release automation.

## Decision

Use a **native DS implementation built on the FAKE-08 core as the bootstrap**, then aggressively optimize the DS-specific host and hot rendering paths.

That gives us:

- a legal/open implementation path,
- a fast route to a shipping artifact,
- and a realistic base for future deeper rewrites.
