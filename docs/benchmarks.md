# Benchmarks

Desktop microbenchmarks comparing the FAKE-08-derived baseline against the new clean native runtime.

| Cart | Fake08 us/frame | Native us/frame | Faster runtime | Speedup |
| --- | ---: | ---: | --- | ---: |
| `fillrate.p8` | 140.31 | 96.75 | native | 1.45x |
| `sprite_stress.p8` | 213.46 | 65.47 | native | 3.26x |

## Emulator smoke notes

- Installed macOS emulators: `melonDS`, `DeSmuME`
- `melonDS` successfully opens both `.nds` files for quick smoke testing
- current observation:
  - FAKE-08-derived baseline reaches its startup text in `melonDS` (`FAT init failed.` with the current emulator filesystem config)
  - the new native runtime currently boots to a black screen in `melonDS`, so desktop benchmark numbers are the current source of truth for performance comparisons while DS-emulator bring-up continues

## Raw output

```text
runtime=fake08 cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=600 us_per_frame=140.31 fps_equivalent=7127.08
runtime=native cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=600 us_per_frame=96.75 fps_equivalent=10335.56
runtime=fake08 cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=600 us_per_frame=213.46 fps_equivalent=4684.83
runtime=native cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=600 us_per_frame=65.47 fps_equivalent=15275.34
```
