# Benchmarks

Desktop microbenchmarks comparing the FAKE-08-derived baseline, the current clean C++ runtime, the Rust runtime called directly from Rust, and the Rust runtime called through a C++ host wrapper.

| Cart | Fake08 us/frame | Native C++ us/frame | Rust via C++ host us/frame | Native Rust us/frame | Fastest runtime | C++ host overhead vs Rust |
| --- | ---: | ---: | ---: | ---: | --- | ---: |
| `api_mix.p8` | 31.68 | 13.44 | 6.48 | 6.30 | native-rs | 1.03x |
| `fillrate.p8` | 126.14 | 83.70 | 42.05 | 42.73 | native-rs-cpp | 0.98x |
| `sprite_stress.p8` | 219.73 | 67.11 | 42.81 | 44.07 | native-rs-cpp | 0.97x |

## Validation notes

- desktop benchmarks remain the current source of truth for runtime-performance comparisons
- Rust status:
  - the Rust runtime is currently a desktop prototype with a minimal Lua bridge and safe Rust core primitives
  - the host-side migration path now includes a C++ wrapper that can drive the Rust runtime through the C ABI using either the existing `dsp::native::Cart` representation or direct Rust cart loading from path/source
  - it is not yet wired into the main DS runtime build, but it is now benchmarkable on desktop both directly from Rust and through a C++ host caller

## Raw output

```text
runtime=fake08 cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=600 us_per_frame=126.14 fps_equivalent=7927.39
runtime=native cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=600 us_per_frame=83.70 fps_equivalent=11946.72
runtime=native-rs-cpp cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=600 us_per_frame=42.05 fps_equivalent=23781.21
runtime=native-rs cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=600 us_per_frame=42.73 fps_equivalent=23401.85
runtime=fake08 cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=600 us_per_frame=219.73 fps_equivalent=4550.97
runtime=native cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=600 us_per_frame=67.11 fps_equivalent=14901.28
runtime=native-rs-cpp cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=600 us_per_frame=42.81 fps_equivalent=23360.85
runtime=native-rs cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=600 us_per_frame=44.07 fps_equivalent=22691.17
runtime=fake08 cart=/Users/danhimebauch/Developer/dspico8/bench/carts/api_mix.p8 frames=600 us_per_frame=31.68 fps_equivalent=31560.68
runtime=native cart=/Users/danhimebauch/Developer/dspico8/bench/carts/api_mix.p8 frames=600 us_per_frame=13.44 fps_equivalent=74404.76
runtime=native-rs-cpp cart=/Users/danhimebauch/Developer/dspico8/bench/carts/api_mix.p8 frames=600 us_per_frame=6.48 fps_equivalent=154241.65
runtime=native-rs cart=/Users/danhimebauch/Developer/dspico8/bench/carts/api_mix.p8 frames=600 us_per_frame=6.30 fps_equivalent=158688.18
```
