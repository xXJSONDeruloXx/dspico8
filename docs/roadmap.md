# Roadmap

## Phase 0 — Research and repo bootstrap

- [x] Survey reference repos
- [x] Write research notes
- [x] Write architecture plan
- [x] Write roadmap
- [ ] Create public repo and push initial docs

## Phase 1 — Bring up a real DS-native build

- [ ] Import the proven FAKE-08 native core pieces needed for DS
- [ ] Import / wire the DS ARM7 + ARM9 platform layer
- [ ] Keep or restore desktop regression tests
- [ ] Build a working `.nds` locally
- [ ] Add basic usage / build docs

## Phase 2 — DS performance pass

- [ ] Replace the current framebuffer upload hot path with a LUT-based expander
- [ ] Tighten compiler flags and remove obvious slow paths in the DS build
- [ ] Identify common renderer hotspots and specialize them
- [ ] Validate input, audio, save data, and stretch modes after optimization

## Phase 3 — Reproducible automation

- [ ] Add Docker-backed local build script(s)
- [ ] Add GitHub Actions for tests + `.nds` builds
- [ ] Upload build artifacts from CI
- [ ] Add tagged prerelease publishing

## Phase 4 — Release candidate

- [ ] Produce a fully built `.nds` artifact
- [ ] Publish it as a GitHub prerelease
- [ ] Document known limitations and next perf targets

## Success criteria

Done means:

- the repo is public and pushed,
- docs exist in `docs/`,
- the project builds a real `.nds`,
- CI can reproduce that build,
- and a prerelease exists with the generated artifact attached.
