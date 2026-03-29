# Benchmarks

Desktop microbenchmarks comparing the FAKE-08-derived baseline, the current clean C++ runtime, and the new Rust prototype runtime.

| Cart | Fake08 us/frame | Native C++ us/frame | Native Rust us/frame | Fastest runtime | Rust vs C++ |
| --- | ---: | ---: | ---: | --- | ---: |
| `fillrate.p8` | 128.70 | 84.64 | 43.20 | native-rs | 1.96x |
| `sprite_stress.p8` | 219.98 | 65.83 | 44.07 | native-rs | 1.49x |

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
runtime=fake08 cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=600 us_per_frame=128.70 fps_equivalent=7770.01
runtime=native cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=600 us_per_frame=84.64 fps_equivalent=11815.44
runtime=native-rs cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=600 us_per_frame=43.20 fps_equivalent=23149.04
runtime=fake08 cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=600 us_per_frame=219.98 fps_equivalent=4545.94
runtime=native cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=600 us_per_frame=65.83 fps_equivalent=15191.41
runtime=native-rs cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=600 us_per_frame=44.07 fps_equivalent=22692.89
```
