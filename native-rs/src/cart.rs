use crate::{MAP_HEIGHT, MAP_WIDTH, SPRITE_SHEET_HEIGHT, SPRITE_SHEET_WIDTH};

use alloc::format;
use alloc::string::String;
#[cfg(feature = "png")]
use alloc::string::ToString;
#[cfg(feature = "png")]
use alloc::vec;
use alloc::vec::Vec;
#[cfg(feature = "png")]
use std::io::Cursor;
#[cfg(feature = "std")]
use std::{fs, path::Path};

#[cfg(feature = "png")]
const LEGACY_COMPRESSION_LUT: &[u8] =
    b"\n 0123456789abcdefghijklmnopqrstuvwxyz!#%(){}[]<>+=/*:;.,~_";
#[cfg(feature = "png")]
const PNG_WIDTH: usize = 160;
#[cfg(feature = "png")]
const PNG_HEIGHT: usize = 205;
#[cfg(feature = "png")]
const ROM_GFX_BYTES: usize = 0x2000;
#[cfg(feature = "png")]
const ROM_MAP_LO_BYTES: usize = 0x1000;
#[cfg(feature = "png")]
const ROM_FLAGS_BYTES: usize = 0x0100;
#[cfg(feature = "png")]
const ROM_MUSIC_BYTES: usize = 0x0100;
#[cfg(feature = "png")]
const ROM_SFX_BYTES: usize = 0x1100;
#[cfg(feature = "png")]
const ROM_CODE_BYTES: usize = 0x3d00;
#[cfg(feature = "png")]
const ROM_TOTAL_BYTES: usize = 0x8001;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Cart {
    pub name: String,
    pub lua: String,
    pub gfx: [u8; SPRITE_SHEET_WIDTH * SPRITE_SHEET_HEIGHT],
    pub map: [u8; MAP_WIDTH * MAP_HEIGHT],
    pub flags: [u8; 256],
}

impl Cart {
    pub fn empty(name: impl Into<String>) -> Self {
        Self {
            name: name.into(),
            lua: String::new(),
            gfx: [0; SPRITE_SHEET_WIDTH * SPRITE_SHEET_HEIGHT],
            map: [0; MAP_WIDTH * MAP_HEIGHT],
            flags: [0; 256],
        }
    }
}

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
enum Section {
    None,
    Lua,
    Gfx,
    Map,
    Gff,
}

#[cfg(feature = "std")]
pub fn load_cart_from_path(path: impl AsRef<Path>) -> Result<Cart, String> {
    let path = path.as_ref();
    let bytes =
        fs::read(path).map_err(|e| format!("failed to read cart {}: {e}", path.display()))?;
    load_cart_from_bytes(path.to_string_lossy().as_ref(), &bytes)
}

pub fn load_cart_from_source(name: &str, source: &str) -> Result<Cart, String> {
    parse_p8_text(name, source)
}

pub fn load_cart_from_bytes(name: &str, bytes: &[u8]) -> Result<Cart, String> {
    if bytes.starts_with(b"\x89PNG") {
        #[cfg(feature = "png")]
        {
            return load_cart_from_png(name, bytes);
        }
        #[cfg(not(feature = "png"))]
        {
            return Err(format!("png carts unsupported in this build for {name}"));
        }
    } else if bytes.starts_with(b"pico-8 cartridge") {
        let source = String::from_utf8_lossy(bytes);
        parse_p8_text(name, &source)
    } else {
        Err(format!("unknown cart format for {name}"))
    }
}

fn parse_p8_text(name: &str, source: &str) -> Result<Cart, String> {
    let mut cart = Cart::empty(name);
    let mut section = Section::None;
    let mut gfx_row = 0usize;
    let mut map_row = 0usize;
    let mut gff_row = 0usize;

    for raw_line in source.lines() {
        let line = raw_line.strip_suffix('\r').unwrap_or(raw_line);

        if line.starts_with("__lua__") {
            section = Section::Lua;
            continue;
        }
        if line.starts_with("__gfx__") {
            section = Section::Gfx;
            continue;
        }
        if line.starts_with("__map__") {
            section = Section::Map;
            continue;
        }
        if line.starts_with("__gff__") {
            section = Section::Gff;
            continue;
        }
        if line.starts_with("__") {
            section = Section::None;
            continue;
        }

        match section {
            Section::Lua => {
                cart.lua.push_str(line);
                cart.lua.push('\n');
            }
            Section::Gfx => {
                parse_gfx_line(line, gfx_row, &mut cart);
                gfx_row += 1;
            }
            Section::Map => {
                parse_map_line(line, map_row, &mut cart);
                map_row += 1;
            }
            Section::Gff => {
                parse_flag_line(line, gff_row, &mut cart);
                gff_row += 1;
            }
            Section::None => {}
        }
    }

    if cart.lua.is_empty() {
        return Err(format!("cart {name} has no __lua__ section"));
    }

    Ok(cart)
}

fn hex_digit_to_int(c: char) -> Option<u8> {
    match c {
        '0'..='9' => Some((c as u8) - b'0'),
        'a'..='f' => Some(10 + (c as u8) - b'a'),
        'A'..='F' => Some(10 + (c as u8) - b'A'),
        _ => None,
    }
}

fn parse_gfx_line(line: &str, row: usize, cart: &mut Cart) {
    if row >= SPRITE_SHEET_HEIGHT {
        return;
    }

    for (x, ch) in line.chars().take(SPRITE_SHEET_WIDTH).enumerate() {
        if let Some(v) = hex_digit_to_int(ch) {
            cart.gfx[row * SPRITE_SHEET_WIDTH + x] = v;
        }
    }
}

fn parse_map_line(line: &str, row: usize, cart: &mut Cart) {
    if row >= MAP_HEIGHT {
        return;
    }

    let chars: Vec<char> = line.chars().collect();
    let byte_count = (chars.len() / 2).min(MAP_WIDTH);
    for x in 0..byte_count {
        if let (Some(hi), Some(lo)) = (
            hex_digit_to_int(chars[x * 2]),
            hex_digit_to_int(chars[x * 2 + 1]),
        ) {
            cart.map[row * MAP_WIDTH + x] = (hi << 4) | lo;
        }
    }
}

fn parse_flag_line(line: &str, row: usize, cart: &mut Cart) {
    let start = row * 128;
    if start >= cart.flags.len() {
        return;
    }

    let chars: Vec<char> = line.chars().collect();
    let byte_count = (chars.len() / 2).min(128);
    for i in 0..byte_count {
        if let (Some(hi), Some(lo)) = (
            hex_digit_to_int(chars[i * 2]),
            hex_digit_to_int(chars[i * 2 + 1]),
        ) {
            let idx = start + i;
            if idx < cart.flags.len() {
                cart.flags[idx] = (hi << 4) | lo;
            }
        }
    }
}

#[cfg(feature = "png")]
fn load_cart_from_png(name: &str, bytes: &[u8]) -> Result<Cart, String> {
    let mut decoder = png::Decoder::new(Cursor::new(bytes));
    decoder.set_transformations(png::Transformations::EXPAND | png::Transformations::STRIP_16);
    let mut reader = decoder
        .read_info()
        .map_err(|e| format!("png decoder error for {name}: {e}"))?;

    let out_size = reader
        .output_buffer_size()
        .ok_or_else(|| format!("png decoder did not report output size for {name}"))?;
    let mut image = vec![0; out_size];
    let info = reader
        .next_frame(&mut image)
        .map_err(|e| format!("png frame decode error for {name}: {e}"))?;
    let pixels = &image[..info.buffer_size()];

    if info.width as usize != PNG_WIDTH || info.height as usize != PNG_HEIGHT {
        return Err(format!(
            "invalid png dimensions for {name}: expected {}x{}, got {}x{}",
            PNG_WIDTH, PNG_HEIGHT, info.width, info.height
        ));
    }

    let rgba = convert_to_rgba8(pixels, &info)?;
    unpack_png_cart(name, &rgba)
}

#[cfg(feature = "png")]
fn convert_to_rgba8(pixels: &[u8], info: &png::OutputInfo) -> Result<Vec<u8>, String> {
    let pixel_count = (info.width as usize) * (info.height as usize);
    let mut out = Vec::with_capacity(pixel_count * 4);

    match info.color_type {
        png::ColorType::Rgba => out.extend_from_slice(pixels),
        png::ColorType::Rgb => {
            for chunk in pixels.chunks_exact(3) {
                out.extend_from_slice(&[chunk[0], chunk[1], chunk[2], 255]);
            }
        }
        png::ColorType::Grayscale => {
            for &v in pixels {
                out.extend_from_slice(&[v, v, v, 255]);
            }
        }
        png::ColorType::GrayscaleAlpha => {
            for chunk in pixels.chunks_exact(2) {
                out.extend_from_slice(&[chunk[0], chunk[0], chunk[0], chunk[1]]);
            }
        }
        png::ColorType::Indexed => {
            return Err("indexed PNG carts are not supported yet".to_string());
        }
    }

    Ok(out)
}

#[cfg(feature = "png")]
fn unpack_png_cart(name: &str, rgba: &[u8]) -> Result<Cart, String> {
    if rgba.len() < PNG_WIDTH * PNG_HEIGHT * 4 {
        return Err(format!("decoded image too small for {name}"));
    }

    let mut rom = vec![0u8; ROM_TOTAL_BYTES];
    for (pixel_idx, chunk) in rgba.chunks_exact(4).enumerate() {
        if pixel_idx >= ROM_TOTAL_BYTES {
            break;
        }

        let r = chunk[0] & 0x03;
        let g = chunk[1] & 0x03;
        let b = chunk[2] & 0x03;
        let a = chunk[3] & 0x03;
        rom[pixel_idx] = (a << 6) | (r << 4) | (g << 2) | b;
    }

    let gfx_bytes = &rom[..ROM_GFX_BYTES];
    let map_lo = &rom[ROM_GFX_BYTES..ROM_GFX_BYTES + ROM_MAP_LO_BYTES];
    let flags =
        &rom[ROM_GFX_BYTES + ROM_MAP_LO_BYTES..ROM_GFX_BYTES + ROM_MAP_LO_BYTES + ROM_FLAGS_BYTES];
    let _music = &rom[ROM_GFX_BYTES + ROM_MAP_LO_BYTES + ROM_FLAGS_BYTES
        ..ROM_GFX_BYTES + ROM_MAP_LO_BYTES + ROM_FLAGS_BYTES + ROM_MUSIC_BYTES];
    let _sfx = &rom[ROM_GFX_BYTES + ROM_MAP_LO_BYTES + ROM_FLAGS_BYTES + ROM_MUSIC_BYTES
        ..ROM_GFX_BYTES + ROM_MAP_LO_BYTES + ROM_FLAGS_BYTES + ROM_MUSIC_BYTES + ROM_SFX_BYTES];
    let code =
        &rom[ROM_GFX_BYTES + ROM_MAP_LO_BYTES + ROM_FLAGS_BYTES + ROM_MUSIC_BYTES + ROM_SFX_BYTES
            ..ROM_GFX_BYTES
                + ROM_MAP_LO_BYTES
                + ROM_FLAGS_BYTES
                + ROM_MUSIC_BYTES
                + ROM_SFX_BYTES
                + ROM_CODE_BYTES];

    let lua_bytes = decode_lua_block(code)?;
    let mut cart = Cart::empty(name);
    cart.lua = String::from_utf8_lossy(&lua_bytes).into_owned();
    if cart.lua.is_empty() {
        return Err(format!("decoded png cart {name} has no lua"));
    }

    for (i, &byte) in gfx_bytes.iter().enumerate() {
        let px = i * 2;
        if px + 1 < cart.gfx.len() {
            cart.gfx[px] = byte & 0x0f;
            cart.gfx[px + 1] = (byte >> 4) & 0x0f;
        }
    }

    cart.map[..ROM_MAP_LO_BYTES].copy_from_slice(map_lo);
    cart.map[ROM_MAP_LO_BYTES..(ROM_MAP_LO_BYTES * 2)].copy_from_slice(&gfx_bytes[0x1000..0x2000]);
    cart.flags.copy_from_slice(flags);
    Ok(cart)
}

#[cfg(feature = "png")]
fn decode_lua_block(code: &[u8]) -> Result<Vec<u8>, String> {
    if code.len() < 8 {
        return Err("lua block too small".to_string());
    }

    if code.starts_with(&[0, b'p', b'x', b'a']) {
        decode_pxa(code)
    } else if code.starts_with(&[b':', b'c', b':', 0]) {
        decode_legacy_compressed(code)
    } else {
        let length = code.iter().position(|&b| b == 0).unwrap_or(code.len());
        Ok(code[..length].to_vec())
    }
}

#[cfg(feature = "png")]
fn decode_legacy_compressed(code: &[u8]) -> Result<Vec<u8>, String> {
    let length = ((code[4] as usize) << 8) | (code[5] as usize);
    let mut out = Vec::with_capacity(length);
    let mut i = 8usize;

    while i < code.len() && out.len() < length {
        let byte = code[i];
        if byte == 0x00 {
            i += 1;
            if i >= code.len() {
                return Err("legacy compressed cart truncated after literal marker".to_string());
            }
            out.push(code[i]);
        } else if byte < 0x3c {
            let idx = (byte - 1) as usize;
            let ch = *LEGACY_COMPRESSION_LUT
                .get(idx)
                .ok_or_else(|| format!("legacy compression LUT lookup out of range: {idx}"))?;
            out.push(ch);
        } else {
            if i + 1 >= code.len() {
                return Err("legacy compressed cart truncated during backreference".to_string());
            }
            let offset = ((byte as usize - 0x3c) * 16) + ((code[i + 1] as usize) & 0x0f);
            let count = ((code[i + 1] as usize) >> 4) + 2;
            if offset == 0 || offset > out.len() {
                return Err(format!(
                    "legacy compression invalid backreference offset: {offset}"
                ));
            }
            let start = out.len() - offset;
            for j in 0..count {
                let value = out[start + j];
                out.push(value);
            }
            i += 1;
        }
        i += 1;
    }

    Ok(out)
}

#[cfg(feature = "png")]
#[derive(Clone)]
struct MoveToFront {
    state: [u8; 256],
}

#[cfg(feature = "png")]
impl MoveToFront {
    fn new() -> Self {
        let mut state = [0u8; 256];
        for (i, slot) in state.iter_mut().enumerate() {
            *slot = i as u8;
        }
        Self { state }
    }

    fn get(&mut self, index: usize) -> u8 {
        let index = index.min(255);
        let value = self.state[index];
        for i in (1..=index).rev() {
            self.state[i] = self.state[i - 1];
        }
        self.state[0] = value;
        value
    }
}

#[cfg(feature = "png")]
struct BitReader<'a> {
    bytes: &'a [u8],
    pos_bits: usize,
    max_bits: usize,
}

#[cfg(feature = "png")]
impl<'a> BitReader<'a> {
    fn new(bytes: &'a [u8], pos_bits: usize, max_bytes: usize) -> Self {
        Self {
            bytes,
            pos_bits,
            max_bits: max_bytes.saturating_mul(8),
        }
    }

    fn get_bits(&mut self, count: usize) -> u32 {
        let mut n = 0u32;
        for i in 0..count {
            if self.pos_bits >= self.max_bits {
                break;
            }
            let bit = (self.bytes[self.pos_bits >> 3] >> (self.pos_bits & 0x7)) & 0x1;
            n |= (bit as u32) << i;
            self.pos_bits += 1;
        }
        n
    }
}

#[cfg(feature = "png")]
fn decode_pxa(code: &[u8]) -> Result<Vec<u8>, String> {
    let length = ((code[4] as usize) << 8) | (code[5] as usize);
    let compressed = ((code[6] as usize) << 8) | (code[7] as usize);
    if compressed > code.len() {
        return Err("pxa compressed size exceeds input length".to_string());
    }

    let mut reader = BitReader::new(code, 8 * 8, compressed);
    let mut mtf = MoveToFront::new();
    let mut out = Vec::with_capacity(length);

    while out.len() < length && reader.pos_bits < reader.max_bits {
        if reader.get_bits(1) != 0 {
            let mut nbits = 4usize;
            while reader.get_bits(1) != 0 {
                nbits += 1;
            }
            let n = reader.get_bits(nbits) as usize + (1usize << nbits) - 16;
            let ch = mtf.get(n);
            if ch == 0 {
                break;
            }
            out.push(ch);
        } else {
            let nbits = if reader.get_bits(1) != 0 {
                if reader.get_bits(1) != 0 {
                    5usize
                } else {
                    10usize
                }
            } else {
                15usize
            };
            let offset = reader.get_bits(nbits) as usize + 1;

            if nbits == 10 && offset == 1 {
                let mut ch = reader.get_bits(8) as u8;
                while ch != 0 {
                    out.push(ch);
                    ch = reader.get_bits(8) as u8;
                }
            } else {
                let mut len = 3usize;
                loop {
                    let n = reader.get_bits(3) as usize;
                    len += n;
                    if n != 7 {
                        break;
                    }
                }

                if offset == 0 || offset > out.len() {
                    return Err(format!("pxa invalid backreference offset: {offset}"));
                }

                for _ in 0..len {
                    let value = out[out.len() - offset];
                    out.push(value);
                }
            }
        }
    }

    Ok(out)
}

#[cfg(all(test, feature = "std"))]
mod tests {
    use super::*;
    use crate::{SCREEN_HEIGHT, SCREEN_WIDTH};

    #[test]
    fn parses_plain_p8_cart() {
        let cart =
            load_cart_from_path("../bench/carts/fillrate.p8").expect("fillrate.p8 should parse");
        assert!(cart.lua.contains("rectfill"));
        assert_eq!(cart.gfx.len(), SPRITE_SHEET_WIDTH * SPRITE_SHEET_HEIGHT);
        assert_eq!(cart.map.len(), MAP_WIDTH * MAP_HEIGHT);
        assert_eq!(cart.flags.len(), 256);
    }

    #[cfg(feature = "png")]
    #[test]
    fn parses_repo_png_cart() {
        let cart = load_cart_from_path("../test/carts/cartparsetest.p8.png")
            .expect("cartparsetest.p8.png should parse");
        assert!(!cart.lua.is_empty());
    }

    #[cfg(feature = "png")]
    #[test]
    fn parses_legacy_repo_png_cart() {
        let cart = load_cart_from_path("../test/carts/test_legacypng_cart.p8.png")
            .expect("legacy png cart should parse");
        assert!(!cart.lua.is_empty());
    }

    #[test]
    fn screen_constants_match_existing_runtime_shape() {
        assert_eq!(SCREEN_WIDTH, 128);
        assert_eq!(SCREEN_HEIGHT, 128);
        assert_eq!(SPRITE_SHEET_WIDTH, 128);
        assert_eq!(SPRITE_SHEET_HEIGHT, 128);
        assert_eq!(MAP_WIDTH, 128);
        assert_eq!(MAP_HEIGHT, 64);
    }
}
