# Hybrid clean-runtime target

This document defines the practical near-term finish line for `dspico8` if the goal is:

- **no FAKE-08 core**
- **Rust for most portable runtime logic**
- **C / C++ allowed at the boundaries for now**

## What “done enough” means

A usable hybrid clean-runtime product should have:

1. a shipped `.nds` artifact that does **not** use the FAKE-08 core
2. Rust owning most portable runtime logic:
   - cart parsing
   - runtime state
   - draw logic
   - framebuffer generation
3. C / C++ limited mainly to:
   - Lua / FFI boundary glue
   - DS startup and platform integration
   - low-level toolchain / linkage glue
4. a meaningful working cart set beyond microbenchmarks
5. reproducible builds, CI, and prerelease artifacts

## Current status

Already true:

- Rust owns the extracted runtime core in `native-rs-core/`
- Rust owns the desktop prototype runtime in `native-rs/`
- a C ABI exists for the Rust runtime
- a thin C++ host wrapper can drive the Rust runtime
- host-side callers can now load carts through unified runtime methods
- the reduced `no_std` / `alloc` Rust runtime build now cross-builds for `armv5te-none-eabi`
- a DS smoke target now links the **full Rust runtime** staticlib, not just `native-rs-core`

Not true yet:

- the main shipped clean DS runtime is still not Rust-backed
- compatibility is still only a subset, not FAKE-08-level breadth
- the old clean C++ runtime cannot be deleted yet

## Next 5 implementation steps

1. **Promote the Rust runtime from smoke target to selectable DS runtime path**
   - move from `platform/nds-rust-runtime/` proof-of-linkage toward the main DS-native flow
   - reuse the now-unified `LoadCartFromPath` / `LoadCartFromSource` runtime interface where possible

2. **Expand shared runtime parity on real carts**
   - add more real-world carts beyond the benchmark set
   - keep framebuffer hashes / deterministic checks wherever possible
   - identify the next missing APIs from actual cart failures

3. **Shift more DS cart-loading/bootstrap responsibility into Rust-backed code**
   - prefer Rust path/source parsing where possible
   - keep C / C++ doing filesystem and platform work only when necessary

4. **Reduce the clean C++ runtime to reference / fallback status**
   - avoid new feature investment there unless it directly unblocks Rust
   - remove host-side and DS-side call sites that no longer need the old class

5. **Swap the main clean DS build over once the Rust path is stable enough**
   - then delete obsolete C++ runtime code in slices
   - keep only boundary glue that is still practically useful

## Explicit non-goals for this phase

These are **not** required before calling the hybrid target a success:

- zero C in the repo
- zero C++ in the repo
- perfect FAKE-08 compatibility on day one
- optional official `pico8.dat` integration
- real hardware for every intermediate milestone

## The real gate

The main gate is still:

> make the real clean DS build use the Rust runtime rather than the clean C++ runtime.

Once that is true, deleting most of the remaining C++ becomes much more mechanical.
