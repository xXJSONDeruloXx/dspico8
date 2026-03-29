# Framebuffer hash comparisons

Framebuffer FNV-1a hashes comparing the current clean C++ runtime against the Rust runtime on shared subset carts.

| Cart | Native C++ hash | Native Rust hash | Match |
| --- | --- | --- | --- |
| `api_mix_det.p8` | `0x77f8999eec4e72bc` | `0x77f8999eec4e72bc` | yes |
| `fillrate.p8` | `0x79425ebc7d845f83` | `0x79425ebc7d845f83` | yes |
| `sprite_stress.p8` | `0x3287c49fcb9d8383` | `0x3287c49fcb9d8383` | yes |

## Raw output

```text
runtime=native-hash cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=120 fnv64=0x79425ebc7d845f83
runtime=native-rs-hash cart=/Users/danhimebauch/Developer/dspico8/bench/carts/fillrate.p8 frames=120 fnv64=0x79425ebc7d845f83
runtime=native-hash cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=120 fnv64=0x3287c49fcb9d8383
runtime=native-rs-hash cart=/Users/danhimebauch/Developer/dspico8/bench/carts/sprite_stress.p8 frames=120 fnv64=0x3287c49fcb9d8383
runtime=native-hash cart=/Users/danhimebauch/Developer/dspico8/bench/carts/api_mix_det.p8 frames=120 fnv64=0x77f8999eec4e72bc
runtime=native-rs-hash cart=/Users/danhimebauch/Developer/dspico8/bench/carts/api_mix_det.p8 frames=120 fnv64=0x77f8999eec4e72bc
```
