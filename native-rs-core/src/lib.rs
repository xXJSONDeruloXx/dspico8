#![no_std]

pub const SCREEN_WIDTH: usize = 128;
pub const SCREEN_HEIGHT: usize = 128;
pub const SPRITE_SHEET_WIDTH: usize = 128;
pub const SPRITE_SHEET_HEIGHT: usize = 128;
pub const MAP_WIDTH: usize = 128;
pub const MAP_HEIGHT: usize = 64;

#[derive(Clone, Debug)]
pub struct RuntimeCore {
    frame_buffer: [u8; SCREEN_WIDTH * SCREEN_HEIGHT],
    sprite_sheet: [u8; SPRITE_SHEET_WIDTH * SPRITE_SHEET_HEIGHT],
    map: [u8; MAP_WIDTH * MAP_HEIGHT],
    flags: [u8; 256],
    color: u8,
    camera_x: i32,
    camera_y: i32,
    clip_x0: i32,
    clip_y0: i32,
    clip_x1: i32,
    clip_y1: i32,
}

impl Default for RuntimeCore {
    fn default() -> Self {
        Self::new()
    }
}

impl RuntimeCore {
    pub fn new() -> Self {
        let mut core = Self {
            frame_buffer: [0; SCREEN_WIDTH * SCREEN_HEIGHT],
            sprite_sheet: [0; SPRITE_SHEET_WIDTH * SPRITE_SHEET_HEIGHT],
            map: [0; MAP_WIDTH * MAP_HEIGHT],
            flags: [0; 256],
            color: 6,
            camera_x: 0,
            camera_y: 0,
            clip_x0: 0,
            clip_y0: 0,
            clip_x1: SCREEN_WIDTH as i32,
            clip_y1: SCREEN_HEIGHT as i32,
        };
        core.reset_draw_state();
        core
    }

    pub fn load_assets(
        &mut self,
        sprite_sheet: &[u8; SPRITE_SHEET_WIDTH * SPRITE_SHEET_HEIGHT],
        map: &[u8; MAP_WIDTH * MAP_HEIGHT],
        flags: &[u8; 256],
    ) {
        self.sprite_sheet.copy_from_slice(sprite_sheet);
        self.map.copy_from_slice(map);
        self.flags.copy_from_slice(flags);
        self.reset_draw_state();
    }

    pub fn reset_draw_state(&mut self) {
        self.color = 6;
        self.camera_x = 0;
        self.camera_y = 0;
        self.clip_x0 = 0;
        self.clip_y0 = 0;
        self.clip_x1 = SCREEN_WIDTH as i32;
        self.clip_y1 = SCREEN_HEIGHT as i32;
        self.frame_buffer.fill(0);
    }

    pub fn frame_buffer(&self) -> &[u8] {
        &self.frame_buffer
    }

    pub fn sprite_sheet(&self) -> &[u8] {
        &self.sprite_sheet
    }

    pub fn map(&self) -> &[u8] {
        &self.map
    }

    pub fn flags(&self) -> &[u8] {
        &self.flags
    }

    pub fn cls(&mut self, color: Option<u8>) {
        self.frame_buffer.fill(color.unwrap_or(0) & 0x0f);
    }

    pub fn color(&mut self, color: Option<u8>) -> u8 {
        let previous = self.color;
        if let Some(color) = color {
            self.color = color & 0x0f;
        }
        previous
    }

    pub fn camera(&mut self, x: Option<i32>, y: Option<i32>) -> (i32, i32) {
        let previous = (self.camera_x, self.camera_y);
        match (x, y) {
            (Some(x), Some(y)) => {
                self.camera_x = x;
                self.camera_y = y;
            }
            _ => {
                self.camera_x = 0;
                self.camera_y = 0;
            }
        }
        previous
    }

    pub fn clip(
        &mut self,
        x: Option<i32>,
        y: Option<i32>,
        w: Option<i32>,
        h: Option<i32>,
    ) -> (i32, i32, i32, i32) {
        let previous = (
            self.clip_x0,
            self.clip_y0,
            self.clip_x1 - self.clip_x0,
            self.clip_y1 - self.clip_y0,
        );

        match (x, y, w, h) {
            (Some(x), Some(y), Some(w), Some(h)) => {
                self.clip_x0 = x.clamp(0, SCREEN_WIDTH as i32);
                self.clip_y0 = y.clamp(0, SCREEN_HEIGHT as i32);
                self.clip_x1 = (x + w).clamp(0, SCREEN_WIDTH as i32);
                self.clip_y1 = (y + h).clamp(0, SCREEN_HEIGHT as i32);
            }
            _ => {
                self.clip_x0 = 0;
                self.clip_y0 = 0;
                self.clip_x1 = SCREEN_WIDTH as i32;
                self.clip_y1 = SCREEN_HEIGHT as i32;
            }
        }

        previous
    }

    pub fn pset(&mut self, x: i32, y: i32, color: Option<u8>) {
        let color = color.unwrap_or(self.color) & 0x0f;
        self.put_pixel(x - self.camera_x, y - self.camera_y, color);
    }

    pub fn pget(&self, x: i32, y: i32) -> u8 {
        self.get_pixel(x - self.camera_x, y - self.camera_y)
    }

    pub fn rectfill(
        &mut self,
        mut x0: i32,
        mut y0: i32,
        mut x1: i32,
        mut y1: i32,
        color: Option<u8>,
    ) {
        x0 -= self.camera_x;
        y0 -= self.camera_y;
        x1 -= self.camera_x;
        y1 -= self.camera_y;
        if x0 > x1 {
            core::mem::swap(&mut x0, &mut x1);
        }
        if y0 > y1 {
            core::mem::swap(&mut y0, &mut y1);
        }
        let color = color.unwrap_or(self.color) & 0x0f;
        for y in y0..=y1 {
            for x in x0..=x1 {
                self.put_pixel(x, y, color);
            }
        }
    }

    pub fn rect(&mut self, mut x0: i32, mut y0: i32, mut x1: i32, mut y1: i32, color: Option<u8>) {
        x0 -= self.camera_x;
        y0 -= self.camera_y;
        x1 -= self.camera_x;
        y1 -= self.camera_y;
        if x0 > x1 {
            core::mem::swap(&mut x0, &mut x1);
        }
        if y0 > y1 {
            core::mem::swap(&mut y0, &mut y1);
        }
        let color = color.unwrap_or(self.color) & 0x0f;
        for x in x0..=x1 {
            self.put_pixel(x, y0, color);
            self.put_pixel(x, y1, color);
        }
        for y in y0..=y1 {
            self.put_pixel(x0, y, color);
            self.put_pixel(x1, y, color);
        }
    }

    pub fn line(&mut self, mut x0: i32, mut y0: i32, mut x1: i32, mut y1: i32, color: Option<u8>) {
        x0 -= self.camera_x;
        y0 -= self.camera_y;
        x1 -= self.camera_x;
        y1 -= self.camera_y;
        let color = color.unwrap_or(self.color) & 0x0f;

        let dx = (x1 - x0).abs();
        let sx = if x0 < x1 { 1 } else { -1 };
        let dy = -(y1 - y0).abs();
        let sy = if y0 < y1 { 1 } else { -1 };
        let mut err = dx + dy;

        loop {
            self.put_pixel(x0, y0, color);
            if x0 == x1 && y0 == y1 {
                break;
            }
            let e2 = err * 2;
            if e2 >= dy {
                err += dy;
                x0 += sx;
            }
            if e2 <= dx {
                err += dx;
                y0 += sy;
            }
        }
    }

    pub fn spr(&mut self, n: u8, dx: i32, dy: i32, w: i32, h: i32, flip_x: bool, flip_y: bool) {
        let dx = dx - self.camera_x;
        let dy = dy - self.camera_y;
        let w = w.max(1);
        let h = h.max(1);
        let sprite_x = ((n as usize) % 16) * 8;
        let sprite_y = ((n as usize) / 16) * 8;

        for sy in 0..(h * 8) {
            for sx in 0..(w * 8) {
                let src_x = sprite_x as i32 + if flip_x { w * 8 - 1 - sx } else { sx };
                let src_y = sprite_y as i32 + if flip_y { h * 8 - 1 - sy } else { sy };
                let color = self.sget(src_x, src_y);
                if color != 0 {
                    self.put_pixel(dx + sx, dy + sy, color);
                }
            }
        }
    }

    pub fn map_draw(&mut self, cel_x: i32, cel_y: i32, sx: i32, sy: i32, cel_w: i32, cel_h: i32) {
        for y in 0..cel_h {
            for x in 0..cel_w {
                let tile = self.mget(cel_x + x, cel_y + y);
                self.spr(tile, sx + x * 8, sy + y * 8, 1, 1, false, false);
            }
        }
    }

    pub fn sget(&self, x: i32, y: i32) -> u8 {
        if x < 0 || y < 0 || x >= SPRITE_SHEET_WIDTH as i32 || y >= SPRITE_SHEET_HEIGHT as i32 {
            return 0;
        }
        self.sprite_sheet[y as usize * SPRITE_SHEET_WIDTH + x as usize]
    }

    pub fn sset(&mut self, x: i32, y: i32, color: u8) {
        if x < 0 || y < 0 || x >= SPRITE_SHEET_WIDTH as i32 || y >= SPRITE_SHEET_HEIGHT as i32 {
            return;
        }
        self.sprite_sheet[y as usize * SPRITE_SHEET_WIDTH + x as usize] = color & 0x0f;
    }

    pub fn mget(&self, x: i32, y: i32) -> u8 {
        if x < 0 || y < 0 || x >= MAP_WIDTH as i32 || y >= MAP_HEIGHT as i32 {
            return 0;
        }
        self.map[y as usize * MAP_WIDTH + x as usize]
    }

    pub fn mset(&mut self, x: i32, y: i32, value: u8) {
        if x < 0 || y < 0 || x >= MAP_WIDTH as i32 || y >= MAP_HEIGHT as i32 {
            return;
        }
        self.map[y as usize * MAP_WIDTH + x as usize] = value;
    }

    pub fn fget(&self, sprite: u8, bit: Option<u8>) -> u8 {
        let value = self.flags[sprite as usize];
        match bit {
            Some(bit) => (value >> (bit & 7)) & 0x1,
            None => value,
        }
    }

    pub fn fset(&mut self, sprite: u8, bit: Option<u8>, value: u8) {
        match bit {
            Some(bit) => {
                let mask = 1u8 << (bit & 7);
                if value != 0 {
                    self.flags[sprite as usize] |= mask;
                } else {
                    self.flags[sprite as usize] &= !mask;
                }
            }
            None => self.flags[sprite as usize] = value,
        }
    }

    fn put_pixel(&mut self, x: i32, y: i32, color: u8) {
        if x < self.clip_x0 || x >= self.clip_x1 || y < self.clip_y0 || y >= self.clip_y1 {
            return;
        }
        self.put_pixel_raw(x, y, color);
    }

    fn put_pixel_raw(&mut self, x: i32, y: i32, color: u8) {
        if x < 0 || y < 0 || x >= SCREEN_WIDTH as i32 || y >= SCREEN_HEIGHT as i32 {
            return;
        }
        self.frame_buffer[y as usize * SCREEN_WIDTH + x as usize] = color & 0x0f;
    }

    fn get_pixel(&self, x: i32, y: i32) -> u8 {
        if x < 0 || y < 0 || x >= SCREEN_WIDTH as i32 || y >= SCREEN_HEIGHT as i32 {
            return 0;
        }
        self.frame_buffer[y as usize * SCREEN_WIDTH + x as usize]
    }
}

#[cfg(test)]
extern crate std;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn rectfill_draws_inside_buffer() {
        let mut core = RuntimeCore::new();
        core.rectfill(0, 0, 3, 3, Some(9));
        assert_eq!(core.pget(0, 0), 9);
        assert_eq!(core.pget(3, 3), 9);
        assert_eq!(core.pget(4, 4), 0);
    }

    #[test]
    fn sprite_zero_is_transparent() {
        let mut core = RuntimeCore::new();
        core.sset(0, 0, 0);
        core.sset(1, 0, 12);
        core.spr(0, 10, 10, 1, 1, false, false);
        assert_eq!(core.pget(10, 10), 0);
        assert_eq!(core.pget(11, 10), 12);
    }

    #[test]
    fn camera_offsets_world_space() {
        let mut core = RuntimeCore::new();
        core.camera(Some(4), Some(2));
        core.pset(4, 2, Some(7));
        assert_eq!(core.frame_buffer()[0], 7);
    }

    #[test]
    fn map_draw_uses_sprite_sheet_tiles() {
        let mut core = RuntimeCore::new();
        core.sset(0, 0, 11);
        core.mset(0, 0, 0);
        core.map_draw(0, 0, 20, 20, 1, 1);
        assert_eq!(core.pget(20, 20), 11);
    }

    #[test]
    fn fset_and_fget_round_trip() {
        let mut core = RuntimeCore::new();
        core.fset(3, Some(2), 1);
        assert_eq!(core.fget(3, Some(2)), 1);
        core.fset(3, None, 0xaa);
        assert_eq!(core.fget(3, None), 0xaa);
    }

    #[test]
    fn clip_prevents_outside_writes() {
        let mut core = RuntimeCore::new();
        core.clip(Some(10), Some(10), Some(10), Some(10));
        core.rectfill(0, 0, 30, 30, Some(12));
        assert_eq!(core.pget(5, 5), 0);
        assert_eq!(core.pget(12, 12), 12);
    }
}
