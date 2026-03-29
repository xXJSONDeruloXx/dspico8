# Benchmarks

Desktop microbenchmarks comparing the FAKE-08-derived baseline, the current clean C++ runtime, the Rust runtime called directly from Rust, and the Rust runtime called through a C++ host wrapper.

| Cart | Fake08 us/frame | Native C++ us/frame | Rust via C++ host us/frame | Native Rust us/frame | Fastest runtime | C++ host overhead vs Rust |
| --- | ---: | ---: | ---: | ---: | --- | ---: |
| `api_mix.p8` | 31.60 | 12.86 | 6.64 | 6.39 | native-rs | 1.04x |
| `fillrate.p8` | 127.11 | 83.85 | 41.15 | 41.04 | native-rs | 1.00x |
| `sprite_stress.p8` | 217.29 | 66.89 | 42.21 | 42.20 | native-rs | 1.00x |

## Validation notes

- desktop benchmarks remain the current source of truth for runtime-performance comparisons
- Rust status:
  - the Rust runtime is currently a desktop prototype with a minimal Lua bridge and safe Rust core primitives
  - the host-side migration path now includes a C++ wrapper that can drive the Rust runtime through the C ABI using either the existing `dsp::native::Cart` representation or direct Rust cart loading from path/source
  - it is not yet wired into the main DS runtime build, but it is now benchmarkable on desktop both directly from Rust and through a C++ host caller

## Raw output

```text
runtime=fake08 cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=600 us_per_frame=127.11 fps_equivalent=7867.51
runtime=native cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=600 us_per_frame=83.85 fps_equivalent=11925.82
runtime=native-rs-cpp cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=600 us_per_frame=41.15 fps_equivalent=24300.35
runtime=native-rs cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=600 us_per_frame=41.04 fps_equivalent=24366.47
runtime=fake08 cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=600 us_per_frame=217.29 fps_equivalent=4602.07
runtime=native cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=600 us_per_frame=66.89 fps_equivalent=14950.66
runtime=native-rs-cpp cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=600 us_per_frame=42.21 fps_equivalent=23692.00
runtime=native-rs cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=600 us_per_frame=42.20 fps_equivalent=23699.49
runtime=fake08 cart=/Users/danhimebauch/Developer/dspico8/bench/carts/api_mix.p8 frames=600 us_per_frame=31.60 fps_equivalent=31648.91
runtime=native cart=/Users/danhimebauch/Developer/dspico8/bench/carts/api_mix.p8 frames=600 us_per_frame=12.86 fps_equivalent=77780.66
runtime=native-rs-cpp cart=/Users/danhimebauch/Developer/dspico8/bench/carts/api_mix.p8 frames=600 us_per_frame=6.64 fps_equivalent=150489.09
runtime=native-rs cart=/Users/danhimebauch/Developer/dspico8/bench/carts/api_mix.p8 frames=600 us_per_frame=6.39 fps_equivalent=156453.72
```
