# Benchmarks

Desktop microbenchmarks comparing the FAKE-08-derived baseline, the current clean C++ runtime, the Rust runtime called directly from Rust, and the Rust runtime called through a C++ host wrapper.

| Cart | Fake08 us/frame | Native C++ us/frame | Rust via C++ host us/frame | Native Rust us/frame | Fastest runtime | C++ host overhead vs Rust |
| --- | ---: | ---: | ---: | ---: | --- | ---: |
| `api_mix.p8` | 31.63 | 12.97 | 6.38 | 6.60 | native-rs-cpp | 0.97x |
| `fillrate.p8` | 126.78 | 79.03 | 41.75 | 40.59 | native-rs | 1.03x |
| `sprite_stress.p8` | 218.71 | 65.55 | 42.70 | 44.01 | native-rs-cpp | 0.97x |

## Validation notes

- desktop benchmarks remain the current source of truth for runtime-performance comparisons
- Rust status:
  - the Rust runtime is currently a desktop prototype with a minimal Lua bridge and safe Rust core primitives
  - the host-side migration path now includes a C++ wrapper that can drive the Rust runtime through the C ABI using the existing `dsp::native::Cart` representation
  - it is not yet wired into the main DS runtime build, but it is now benchmarkable on desktop both directly from Rust and through a C++ host caller

## Raw output

```text
runtime=fake08 cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=600 us_per_frame=126.78 fps_equivalent=7887.58
runtime=native cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=600 us_per_frame=79.03 fps_equivalent=12653.69
runtime=native-rs-cpp cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=600 us_per_frame=41.75 fps_equivalent=23954.01
runtime=native-rs cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=600 us_per_frame=40.59 fps_equivalent=24633.58
runtime=fake08 cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=600 us_per_frame=218.71 fps_equivalent=4572.33
runtime=native cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=600 us_per_frame=65.55 fps_equivalent=15254.75
runtime=native-rs-cpp cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=600 us_per_frame=42.70 fps_equivalent=23418.29
runtime=native-rs cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=600 us_per_frame=44.01 fps_equivalent=22721.25
runtime=fake08 cart=/Users/danhimebauch/Developer/dspico8/bench/carts/api_mix.p8 frames=600 us_per_frame=31.63 fps_equivalent=31618.89
runtime=native cart=/Users/danhimebauch/Developer/dspico8/bench/carts/api_mix.p8 frames=600 us_per_frame=12.97 fps_equivalent=77110.91
runtime=native-rs-cpp cart=/Users/danhimebauch/Developer/dspico8/bench/carts/api_mix.p8 frames=600 us_per_frame=6.38 fps_equivalent=156698.88
runtime=native-rs cart=/Users/danhimebauch/Developer/dspico8/bench/carts/api_mix.p8 frames=600 us_per_frame=6.60 fps_equivalent=151476.90
```
