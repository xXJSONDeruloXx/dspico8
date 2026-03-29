# Official PICO-8 distribution investigation

## Scope

This document records what we learned from the user-supplied official archive:

- `/Users/danhimebauch/Downloads/PICO8dat.dyn.7z`

The goal is to understand whether the official paid distribution can help a Nintendo DS target **without** redistributing proprietary code or data.

## Files observed

Archive contents:

- `pico8.dat`
- `pico8_64`
- `pico8_dyn`

Local hashes from the supplied archive extraction:

- `pico8.dat`: `fcc02a7af4de54abe5a5aaca4bb7fed5c14f62ef295745e3664191e32849576e`
- `pico8_64`: `a2189ef2c500d2d2d79e1e4358c3176f16f89505cf779d671d757de3163c4f7e`
- `pico8_dyn`: `8f8932691293e7ac98e885efae53827d4afaa1b1e994431735037e343449d7c0`

## Binary observations

### `pico8_64`

`file` reports:

- ELF 64-bit
- ARM aarch64
- Linux
- dynamically linked
- not stripped

### `pico8_dyn`

`file` reports:

- ELF 32-bit
- ARM EABI5
- Linux
- dynamically linked
- not stripped

`objdump -p` / symbol inspection show:

- SDL2 dependency
- libc / libm dependency
- references to `pico8.dat`
- references to Lua
- references to cart loading, save data, BBS carts, and `.p8.png`

## Data-file observations

`pico8.dat` is **not** a single executable blob. It appears to be a custom container / pod-style archive with records such as:

- `CPODD`
- `CFIL`
- `cFIL`
- `CBMP`

Observed embedded names / paths include:

- `pod/pico8_boot.p8`
- `pod/gfx1.pod`
- `pod/f_pico8.pod`
- `pod/f_demos.pod`
- `hello.p8`
- `api.p8`
- `auto2d.p8`
- `bounce.p8`
- `cast.p8`
- `sort.p8`
- `wander.p8`
- `waves.p8`
- `src/pico8.js`
- `src/pico8_wasm.js`
- `src/pico8_wasm.wasm`
- `bin/pico8.exe`
- `bin/pico8_dyn.amd64`
- `builds/osx_builds/pico8_player`
- `builds/pi_builds/pico8_player`

There are also strings for:

- built-in demo / API carts
- HTML / JS / WASM assets
- platform build artifacts
- BBS / cartridge update flows

## What this means technically

The supplied official package is useful as a **reference corpus** and may contain useful **user-supplied data assets**, but it does **not** remove the need for our own DS runtime.

Why:

1. the supplied executables target **Linux + SDL2**, not Nintendo DS hardware
2. `pico8.dat` contains resources / packaged assets, but the engine logic still lives in proprietary executables and packaged binaries
3. a DS build cannot simply “run the official binary”
4. a direct translation of proprietary engine code into our repo would not fit the project’s clean implementation goal

## Feasible use of user-supplied official files

The most realistic path is an **optional user-supplied compatibility/resources mode**:

- user places official files on their own SD card / filesystem
- for example under `/pico8/`
- we detect files such as:
  - `/pico8/pico8.dat`
  - optionally other user-provided official assets later

Possible legitimate uses of those files from our runtime:

- inspect packaged resources
- load built-in carts or assets when the user supplies them
- compare behavior against official content during local development
- use them as a black-box compatibility reference

## What we should not do

- commit official extracted content into the repo
- publish official binaries or data in releases
- derive a shipped runtime by copying proprietary implementation code

## Recommendation

Use the official package in two limited ways:

1. **investigation / compatibility reference**
2. **optional user-supplied runtime data source** if we find a clean, technically useful subset

But keep the shipping runtime itself:

- clean
- open
- DS-native
- and now preferably **Rust-first** for new work
