# Benchmarks

Desktop microbenchmarks comparing the FAKE-08-derived baseline, the current clean C++ runtime, and the new Rust prototype runtime.

| Cart | Fake08 us/frame | Native C++ us/frame | Native Rust us/frame | Fastest runtime | Rust vs C++ |
| --- | ---: | ---: | ---: | --- | ---: |
| `api_mix.p8` | 32.81 | 12.24 | 6.75 | native-rs | 1.81x |
| `fillrate.p8` | 127.65 | 82.15 | 40.99 | native-rs | 2.00x |
| `sprite_stress.p8` | 220.20 | 65.03 | 45.09 | native-rs | 1.44x |

## Emulator smoke notes

- Installed macOS emulators: `melonDS`, `DeSmuME`
- `melonDS` successfully opens both `.nds` files for quick smoke testing
- current observation:
  - FAKE-08-derived baseline reaches its startup text in `melonDS` (`FAT init failed.` with the current emulator filesystem config)
  - the new native runtime currently boots to a black screen in `melonDS`, so desktop benchmark numbers remain the current source of truth for performance comparisons while DS-emulator bring-up continues
- Rust status:
  - the Rust runtime is currently a desktop prototype with a minimal Lua bridge and safe Rust core primitives
  - it is not yet wired into the DS build, but it is now benchmarkable on desktop against the existing C++ runtime

## Raw output

```text
runtime=fake08 cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=600 us_per_frame=127.65 fps_equivalent=7833.72
runtime=native cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=600 us_per_frame=82.15 fps_equivalent=12173.35
runtime=native-rs cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=600 us_per_frame=40.99 fps_equivalent=24394.21
runtime=fake08 cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=600 us_per_frame=220.20 fps_equivalent=4541.33
runtime=native cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=600 us_per_frame=65.03 fps_equivalent=15377.52
runtime=native-rs cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=600 us_per_frame=45.09 fps_equivalent=22175.41
runtime=fake08 cart=/Users/danhimebauch/Developer/dspico8/bench/carts/api_mix.p8 frames=600 us_per_frame=32.81 fps_equivalent=30481.61
runtime=native cart=/Users/danhimebauch/Developer/dspico8/bench/carts/api_mix.p8 frames=600 us_per_frame=12.24 fps_equivalent=81710.47
runtime=native-rs cart=/Users/danhimebauch/Developer/dspico8/bench/carts/api_mix.p8 frames=600 us_per_frame=6.75 fps_equivalent=148111.58
```
