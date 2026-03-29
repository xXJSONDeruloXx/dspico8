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

## Phase 2 — Clean native runtime track

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

## Phase 5 — Compatibility expansion

- [ ] Improve `.p8` cart compatibility beyond the current subset
- [ ] Add `.p8.png` cart loading
- [ ] Implement more PICO-8 API coverage
- [ ] Add save-data, audio, and broader cart UX in the clean runtime

## Phase 6 — Release candidate

- [x] Produce fully built `.nds` artifacts
- [x] Publish a GitHub prerelease
- [x] Attach generated artifacts to the prerelease
- [ ] Promote the clean native runtime as the default release once emulator/hardware smoke tests are solid

## Success criteria

The project is on the right track when:

- the repo is public and pushed,
- both baseline and clean native DS builds are reproducible,
- benchmarks show where the new runtime is winning or losing,
- and prereleases ship the generated `.nds` files for comparison.
