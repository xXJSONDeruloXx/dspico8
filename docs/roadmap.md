# Roadmap

## Phase 0 — Research and repo bootstrap

- [x] Survey reference repos
- [x] Write research notes
- [x] Write architecture plan
- [x] Write roadmap
- [x] Create public repo and push initial docs

## Phase 1 — Baseline DS bootstrap

- [x] Import the proven FAKE-08-derived DS baseline for reference
- [x] Keep desktop regression tests for the baseline runtime
- [x] Build a working baseline `.nds` locally
- [x] Add CI + prerelease publishing for the baseline artifact

## Phase 2 — First clean native runtime

- [x] Start a non-FAKE-08 runtime in `native/`
- [x] Use a DS-friendly 8bpp framebuffer in the clean runtime
- [x] Add a DS-native build target in `platform/nds-native/`
- [x] Build a working clean-runtime `.nds` locally
- [x] Keep the baseline build as `make nds-baseline` for comparison

## Phase 3 — Progressive performance comparison

- [x] Add repeatable desktop benchmark carts
- [x] Add a benchmark runner comparing baseline vs native runtime
- [x] Record current results in `docs/benchmarks.md`
- [ ] Expand benchmark coverage to more real-world carts and API mixes

## Phase 4 — Emulator and hardware validation

- [x] Install macOS DS emulators for smoke testing
- [x] Smoke test baseline and native artifacts in `melonDS`
- [ ] Fix the current native-runtime black-screen issue observed in `melonDS`
- [ ] Validate on real hardware / flashcart when available

## Phase 5 — Official PICO-8 investigation

- [x] Inspect the user-supplied official archive contents
- [x] Confirm the supplied binaries target Linux/SDL rather than DS hardware
- [x] Confirm `pico8.dat` is a packaged resource/container file, not a drop-in DS runtime
- [x] Document a safe optional user-supplied official-data path in `docs/official-pico8-investigation.md`
- [ ] Decide whether any clean optional `/pico8/pico8.dat` resource integration is worth keeping long-term

## Phase 6 — Rust-first native runtime foundations

- [x] Install a Rust toolchain locally
- [x] Create `native-rs/`
- [x] Add safe Rust `.p8` parsing
- [x] Add safe Rust `.p8.png` parsing
- [x] Add safe Rust framebuffer / sprite / map primitives
- [x] Add Rust probes for carts and `pico8.dat`
- [x] Add Rust tests to CI

## Phase 7 — Rust runtime execution parity

- [x] Add a minimal Lua bridge with tightly contained unsafe FFI
- [ ] Reach parity with the current clean C++ runtime subset
- [x] Add benchmarkable Rust runtime stepping on desktop
- [x] Compare Rust runtime performance against the current C++ clean runtime

## Phase 8 — Rust DS integration

- [ ] Compile the Rust runtime core into the DS build
- [ ] Keep ARM7 / ARM9 platform glue minimal while swapping in Rust core pieces
- [ ] Boot the Rust-backed native runtime in emulator
- [ ] Validate on hardware once the emulator path is stable

## Phase 9 — Compatibility expansion

- [ ] Improve cart compatibility beyond the current subset
- [ ] Add save-data, audio, and broader cart UX in the clean runtime
- [ ] Decide whether optional user-supplied official data improves compatibility enough to justify support

## Phase 10 — Release candidate

- [x] Produce fully built `.nds` artifacts
- [x] Publish a GitHub prerelease
- [x] Attach generated artifacts to the prerelease
- [ ] Promote the clean native runtime as the default release once emulator/hardware smoke tests are solid
- [ ] Move the preferred clean implementation base from C++ reference code to the Rust runtime

## Success criteria

The project is on the right track when:

- the repo is public and pushed,
- both baseline and clean DS builds are reproducible,
- Rust foundations are covered by tests,
- benchmarks show where the clean runtime is winning or losing,
- and prereleases ship the generated `.nds` files for comparison.
