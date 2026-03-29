use crate::{Cart, RuntimeCore};

use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};
use std::ptr;

#[allow(non_camel_case_types)]
type lua_Integer = i64;
#[allow(non_camel_case_types)]
type lua_Number = f64;
#[allow(non_camel_case_types)]
type lua_KContext = isize;
#[allow(non_camel_case_types)]
type lua_CFunction = Option<unsafe extern "C" fn(*mut lua_State) -> c_int>;
#[allow(non_camel_case_types)]
type lua_KFunction = Option<unsafe extern "C" fn(*mut lua_State, c_int, lua_KContext) -> c_int>;

#[repr(C)]
pub struct lua_State {
    _private: [u8; 0],
}

const LUA_OK: c_int = 0;
const LUA_REGISTRYINDEX: c_int = -1001000;
const LUA_TNIL: c_int = 0;
const LUA_TBOOLEAN: c_int = 1;
const LUA_TFUNCTION: c_int = 6;
static RUNTIME_KEY: u8 = 0;

extern "C" {
    fn luaL_newstate() -> *mut lua_State;
    fn luaL_openlibs(l: *mut lua_State);
    fn lua_close(l: *mut lua_State);
    fn luaL_loadstring(l: *mut lua_State, s: *const c_char) -> c_int;
    fn lua_pcallk(
        l: *mut lua_State,
        nargs: c_int,
        nresults: c_int,
        errfunc: c_int,
        ctx: lua_KContext,
        k: lua_KFunction,
    ) -> c_int;
    fn lua_getglobal(l: *mut lua_State, name: *const c_char) -> c_int;
    fn lua_setglobal(l: *mut lua_State, name: *const c_char);
    fn lua_settop(l: *mut lua_State, idx: c_int);
    fn lua_gettop(l: *mut lua_State) -> c_int;
    fn lua_type(l: *mut lua_State, idx: c_int) -> c_int;
    fn lua_pushinteger(l: *mut lua_State, n: lua_Integer);
    fn lua_pushnumber(l: *mut lua_State, n: lua_Number);
    fn lua_pushboolean(l: *mut lua_State, b: c_int);
    fn lua_pushcclosure(l: *mut lua_State, f: lua_CFunction, n: c_int);
    fn lua_rawgetp(l: *mut lua_State, idx: c_int, p: *const std::ffi::c_void) -> c_int;
    fn lua_rawsetp(l: *mut lua_State, idx: c_int, p: *const std::ffi::c_void);
    fn lua_toboolean(l: *mut lua_State, idx: c_int) -> c_int;
    fn lua_tointegerx(l: *mut lua_State, idx: c_int, isnum: *mut c_int) -> lua_Integer;
    fn lua_tonumberx(l: *mut lua_State, idx: c_int, isnum: *mut c_int) -> lua_Number;
    fn lua_tolstring(l: *mut lua_State, idx: c_int, len: *mut usize) -> *const c_char;
}

fn lua_pop(l: *mut lua_State, n: c_int) {
    unsafe {
        lua_settop(l, -n - 1);
    }
}

fn lua_pcall(l: *mut lua_State, nargs: c_int, nresults: c_int, errfunc: c_int) -> c_int {
    unsafe { lua_pcallk(l, nargs, nresults, errfunc, 0, None) }
}

unsafe fn lua_tostring_ptr(l: *mut lua_State, idx: c_int) -> *const c_char {
    lua_tolstring(l, idx, ptr::null_mut())
}

fn lua_register(l: *mut lua_State, name: &CStr, f: lua_CFunction) {
    unsafe {
        lua_pushcclosure(l, f, 0);
        lua_setglobal(l, name.as_ptr());
    }
}

fn is_none_or_nil(l: *mut lua_State, idx: c_int) -> bool {
    unsafe { lua_type(l, idx) <= LUA_TNIL }
}

fn opt_integer(l: *mut lua_State, idx: c_int) -> Option<i32> {
    if is_none_or_nil(l, idx) {
        None
    } else {
        let mut is_num = 0;
        let value = unsafe { lua_tointegerx(l, idx, &mut is_num) };
        if is_num == 0 {
            None
        } else {
            Some(value as i32)
        }
    }
}

fn opt_number(l: *mut lua_State, idx: c_int) -> Option<f64> {
    if is_none_or_nil(l, idx) {
        None
    } else {
        let mut is_num = 0;
        let value = unsafe { lua_tonumberx(l, idx, &mut is_num) };
        if is_num == 0 {
            None
        } else {
            Some(value)
        }
    }
}

fn get_runtime(l: *mut lua_State) -> &'static mut LuaRuntime {
    unsafe {
        lua_rawgetp(
            l,
            LUA_REGISTRYINDEX,
            &RUNTIME_KEY as *const _ as *const std::ffi::c_void,
        );
        let ptr = lua_tointegerx(l, -1, ptr::null_mut());
        lua_pop(l, 1);
        &mut *(ptr as *mut LuaRuntime)
    }
}

fn set_runtime(l: *mut lua_State, runtime: *mut LuaRuntime) {
    unsafe {
        lua_pushinteger(l, runtime as isize as lua_Integer);
        lua_rawsetp(
            l,
            LUA_REGISTRYINDEX,
            &RUNTIME_KEY as *const _ as *const std::ffi::c_void,
        );
    }
}

unsafe extern "C" fn api_cls(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    runtime.core.cls(opt_integer(l, 1).map(|v| v as u8));
    0
}

unsafe extern "C" fn api_color(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    let previous = runtime.core.color(opt_integer(l, 1).map(|v| v as u8));
    lua_pushinteger(l, previous as lua_Integer);
    1
}

unsafe extern "C" fn api_pset(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    let x = opt_integer(l, 1).unwrap_or(0);
    let y = opt_integer(l, 2).unwrap_or(0);
    let color = opt_integer(l, 3).map(|v| v as u8);
    runtime.core.pset(x, y, color);
    0
}

unsafe extern "C" fn api_pget(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    let x = opt_integer(l, 1).unwrap_or(0);
    let y = opt_integer(l, 2).unwrap_or(0);
    lua_pushinteger(l, runtime.core.pget(x, y) as lua_Integer);
    1
}

unsafe extern "C" fn api_rectfill(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    let x0 = opt_integer(l, 1).unwrap_or(0);
    let y0 = opt_integer(l, 2).unwrap_or(0);
    let x1 = opt_integer(l, 3).unwrap_or(0);
    let y1 = opt_integer(l, 4).unwrap_or(0);
    let color = opt_integer(l, 5).map(|v| v as u8);
    runtime.core.rectfill(x0, y0, x1, y1, color);
    0
}

unsafe extern "C" fn api_rect(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    let x0 = opt_integer(l, 1).unwrap_or(0);
    let y0 = opt_integer(l, 2).unwrap_or(0);
    let x1 = opt_integer(l, 3).unwrap_or(0);
    let y1 = opt_integer(l, 4).unwrap_or(0);
    let color = opt_integer(l, 5).map(|v| v as u8);
    runtime.core.rect(x0, y0, x1, y1, color);
    0
}

unsafe extern "C" fn api_line(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    let x0 = opt_integer(l, 1).unwrap_or(0);
    let y0 = opt_integer(l, 2).unwrap_or(0);
    let x1 = opt_integer(l, 3).unwrap_or(0);
    let y1 = opt_integer(l, 4).unwrap_or(0);
    let color = opt_integer(l, 5).map(|v| v as u8);
    runtime.core.line(x0, y0, x1, y1, color);
    0
}

unsafe extern "C" fn api_spr(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    let n = opt_integer(l, 1).unwrap_or(0) as u8;
    let x = opt_integer(l, 2).unwrap_or(0);
    let y = opt_integer(l, 3).unwrap_or(0);
    let w = opt_number(l, 4).unwrap_or(1.0).max(1.0) as i32;
    let h = opt_number(l, 5).unwrap_or(1.0).max(1.0) as i32;
    let flip_x = unsafe { lua_toboolean(l, 6) != 0 };
    let flip_y = unsafe { lua_toboolean(l, 7) != 0 };
    runtime.core.spr(n, x, y, w, h, flip_x, flip_y);
    0
}

unsafe extern "C" fn api_sget(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    let x = opt_integer(l, 1).unwrap_or(0);
    let y = opt_integer(l, 2).unwrap_or(0);
    lua_pushinteger(l, runtime.core.sget(x, y) as lua_Integer);
    1
}

unsafe extern "C" fn api_sset(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    let x = opt_integer(l, 1).unwrap_or(0);
    let y = opt_integer(l, 2).unwrap_or(0);
    let color = opt_integer(l, 3).unwrap_or(0) as u8;
    runtime.core.sset(x, y, color);
    0
}

unsafe extern "C" fn api_mget(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    let x = opt_integer(l, 1).unwrap_or(0);
    let y = opt_integer(l, 2).unwrap_or(0);
    lua_pushinteger(l, runtime.core.mget(x, y) as lua_Integer);
    1
}

unsafe extern "C" fn api_mset(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    let x = opt_integer(l, 1).unwrap_or(0);
    let y = opt_integer(l, 2).unwrap_or(0);
    let value = opt_integer(l, 3).unwrap_or(0) as u8;
    runtime.core.mset(x, y, value);
    0
}

unsafe extern "C" fn api_map(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    let cel_x = opt_integer(l, 1).unwrap_or(0);
    let cel_y = opt_integer(l, 2).unwrap_or(0);
    let sx = opt_integer(l, 3).unwrap_or(0);
    let sy = opt_integer(l, 4).unwrap_or(0);
    let cel_w = opt_integer(l, 5).unwrap_or(0);
    let cel_h = opt_integer(l, 6).unwrap_or(0);
    runtime.core.map_draw(cel_x, cel_y, sx, sy, cel_w, cel_h);
    0
}

unsafe extern "C" fn api_camera(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    let (prev_x, prev_y) = runtime.core.camera(opt_integer(l, 1), opt_integer(l, 2));
    lua_pushinteger(l, prev_x as lua_Integer);
    lua_pushinteger(l, prev_y as lua_Integer);
    2
}

unsafe extern "C" fn api_clip(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    let (x, y, w, h) = runtime.core.clip(
        opt_integer(l, 1),
        opt_integer(l, 2),
        opt_integer(l, 3),
        opt_integer(l, 4),
    );
    lua_pushinteger(l, x as lua_Integer);
    lua_pushinteger(l, y as lua_Integer);
    lua_pushinteger(l, w as lua_Integer);
    lua_pushinteger(l, h as lua_Integer);
    4
}

unsafe extern "C" fn api_time(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    lua_pushnumber(l, runtime.time_seconds as lua_Number);
    1
}

unsafe extern "C" fn api_t(l: *mut lua_State) -> c_int {
    api_time(l)
}

unsafe extern "C" fn api_flip(_: *mut lua_State) -> c_int {
    0
}

unsafe extern "C" fn api_print(l: *mut lua_State) -> c_int {
    let text = lua_tostring_ptr(l, 1);
    let len = if text.is_null() {
        0
    } else {
        CStr::from_ptr(text).to_bytes().len()
    };
    lua_pushinteger(l, (len * 4) as lua_Integer);
    1
}

unsafe extern "C" fn api_fget(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    let sprite = opt_integer(l, 1).unwrap_or(0) as u8;
    if let Some(bit) = opt_integer(l, 2) {
        let bit = (bit & 7) as u8;
        lua_pushboolean(l, (runtime.core.fget(sprite, Some(bit)) != 0) as c_int);
    } else {
        lua_pushinteger(l, runtime.core.fget(sprite, None) as lua_Integer);
    }
    1
}

unsafe extern "C" fn api_fset(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    let sprite = opt_integer(l, 1).unwrap_or(0) as u8;
    let top = unsafe { lua_gettop(l) };
    if top >= 3 && unsafe { lua_type(l, 3) } == LUA_TBOOLEAN {
        let bit = (opt_integer(l, 2).unwrap_or(0) & 7) as u8;
        let value = unsafe { lua_toboolean(l, 3) != 0 };
        runtime
            .core
            .fset(sprite, Some(bit), if value { 1 } else { 0 });
    } else {
        let value = opt_integer(l, 2).unwrap_or(0) as u8;
        runtime.core.fset(sprite, None, value);
    }
    0
}

unsafe extern "C" fn api_btn(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    if is_none_or_nil(l, 1) {
        lua_pushinteger(l, runtime.input.held as lua_Integer);
        return 1;
    }

    let bit = opt_integer(l, 1).unwrap_or(0);
    if bit < 0 {
        lua_pushboolean(l, 0);
    } else {
        lua_pushboolean(
            l,
            (((runtime.input.held >> (bit as u8 & 7)) & 0x1) != 0) as c_int,
        );
    }
    1
}

unsafe extern "C" fn api_btnp(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    if is_none_or_nil(l, 1) {
        lua_pushinteger(l, runtime.input.down as lua_Integer);
        return 1;
    }

    let bit = opt_integer(l, 1).unwrap_or(0);
    if bit < 0 {
        lua_pushboolean(l, 0);
    } else {
        lua_pushboolean(
            l,
            (((runtime.input.down >> (bit as u8 & 7)) & 0x1) != 0) as c_int,
        );
    }
    1
}

unsafe extern "C" fn api_rnd(l: *mut lua_State) -> c_int {
    let runtime = get_runtime(l);
    let unit = runtime.next_random();
    let value = match opt_number(l, 1) {
        Some(bound) => unit * bound,
        None => unit,
    };
    lua_pushnumber(l, value as lua_Number);
    1
}

unsafe extern "C" fn api_pal(_: *mut lua_State) -> c_int {
    0
}

unsafe extern "C" fn api_palt(_: *mut lua_State) -> c_int {
    0
}

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq)]
pub struct InputState {
    pub down: u8,
    pub held: u8,
}

pub struct LuaRuntime {
    lua: *mut lua_State,
    core: RuntimeCore,
    input: InputState,
    time_seconds: f64,
    frame_count: u64,
    rng_state: u64,
}

impl Default for LuaRuntime {
    fn default() -> Self {
        Self::new()
    }
}

impl Drop for LuaRuntime {
    fn drop(&mut self) {
        self.reset_lua();
    }
}

impl LuaRuntime {
    pub fn new() -> Self {
        Self {
            lua: ptr::null_mut(),
            core: RuntimeCore::new(),
            input: InputState::default(),
            time_seconds: 0.0,
            frame_count: 0,
            rng_state: 0xD5C1C08u64,
        }
    }

    pub fn frame_buffer(&self) -> &[u8] {
        self.core.frame_buffer()
    }

    pub fn frame_count(&self) -> u64 {
        self.frame_count
    }

    pub fn core(&self) -> &RuntimeCore {
        &self.core
    }

    pub fn load_cart(&mut self, cart: &Cart) -> Result<(), String> {
        self.reset_lua();
        self.core.load_assets(&cart.gfx, &cart.map, &cart.flags);
        self.input = InputState::default();
        self.time_seconds = 0.0;
        self.frame_count = 0;
        self.rng_state = 0xD5C1C08u64;

        let state = unsafe { luaL_newstate() };
        if state.is_null() {
            return Err("failed to create lua state".to_string());
        }
        self.lua = state;

        unsafe {
            luaL_openlibs(self.lua);
        }
        set_runtime(self.lua, self as *mut LuaRuntime);
        self.register_api();

        let helpers = CString::new(
            r#"
flr = math.floor
ceil = math.ceil
min = math.min
max = math.max
abs = math.abs
mid = function(a,b,c)
  local x, y, z = a, b, c
  if x > y then x, y = y, x end
  if y > z then y, z = z, y end
  if x > y then x, y = y, x end
  return y
end
add = function(t, v)
  t[#t + 1] = v
  return v
end
count = function(t)
  return #t
end
del = function(t, v)
  for i = 1, #t do
    if t[i] == v then
      table.remove(t, i)
      return v
    end
  end
  return nil
end
foreach = function(t, fn)
  for i = 1, #t do fn(t[i]) end
end
all = function(t)
  local i = 0
  return function()
    i = i + 1
    return t[i]
  end
end
"#,
        )
        .map_err(|e| format!("failed to build helper source: {e}"))?;
        self.do_string(&helpers)?;

        let cart_source = CString::new(cart.lua.as_str())
            .map_err(|_| "cart source contains interior NUL byte".to_string())?;
        self.do_string(&cart_source)?;
        self.call_if_exists("_init")?;
        Ok(())
    }

    pub fn step(&mut self, time_seconds: f64) -> Result<(), String> {
        self.step_with_input(InputState::default(), time_seconds)
    }

    pub fn step_with_input(&mut self, input: InputState, time_seconds: f64) -> Result<(), String> {
        self.input = input;
        self.time_seconds = time_seconds;
        self.frame_count += 1;

        if !self.call_if_exists("_update60")? {
            self.call_if_exists("_update")?;
        }
        self.call_if_exists("_draw")?;
        Ok(())
    }

    fn next_random(&mut self) -> f64 {
        let mut x = self.rng_state;
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        self.rng_state = x;
        let mantissa = (x >> 11) as f64;
        mantissa / ((1u64 << 53) as f64)
    }

    fn reset_lua(&mut self) {
        if !self.lua.is_null() {
            unsafe {
                lua_close(self.lua);
            }
            self.lua = ptr::null_mut();
        }
    }

    fn register_api(&mut self) {
        macro_rules! reg {
            ($name:literal, $func:expr) => {
                lua_register(
                    self.lua,
                    CStr::from_bytes_with_nul(concat!($name, "\0").as_bytes()).unwrap(),
                    Some($func),
                );
            };
        }

        reg!("cls", api_cls);
        reg!("color", api_color);
        reg!("pset", api_pset);
        reg!("pget", api_pget);
        reg!("rectfill", api_rectfill);
        reg!("rect", api_rect);
        reg!("line", api_line);
        reg!("spr", api_spr);
        reg!("sget", api_sget);
        reg!("sset", api_sset);
        reg!("mget", api_mget);
        reg!("mset", api_mset);
        reg!("map", api_map);
        reg!("fget", api_fget);
        reg!("fset", api_fset);
        reg!("camera", api_camera);
        reg!("clip", api_clip);
        reg!("time", api_time);
        reg!("t", api_t);
        reg!("flip", api_flip);
        reg!("print", api_print);
        reg!("btn", api_btn);
        reg!("btnp", api_btnp);
        reg!("rnd", api_rnd);
        reg!("pal", api_pal);
        reg!("palt", api_palt);
    }

    fn do_string(&mut self, source: &CStr) -> Result<(), String> {
        let status = unsafe { luaL_loadstring(self.lua, source.as_ptr()) };
        if status != LUA_OK {
            return Err(self.take_error_string());
        }
        if lua_pcall(self.lua, 0, 0, 0) != LUA_OK {
            return Err(self.take_error_string());
        }
        Ok(())
    }

    fn call_if_exists(&mut self, function_name: &str) -> Result<bool, String> {
        let name = CString::new(function_name)
            .map_err(|_| format!("invalid function name: {function_name}"))?;
        unsafe {
            lua_getglobal(self.lua, name.as_ptr());
            if lua_type(self.lua, -1) != LUA_TFUNCTION {
                lua_pop(self.lua, 1);
                return Ok(false);
            }
        }
        if lua_pcall(self.lua, 0, 0, 0) != LUA_OK {
            return Err(self.take_error_string());
        }
        Ok(true)
    }

    fn take_error_string(&mut self) -> String {
        unsafe {
            let ptr = lua_tostring_ptr(self.lua, -1);
            let message = if ptr.is_null() {
                "unknown lua error".to_string()
            } else {
                CStr::from_ptr(ptr).to_string_lossy().into_owned()
            };
            lua_pop(self.lua, 1);
            message
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{load_cart_from_path, load_cart_from_source};

    #[test]
    fn rust_lua_runtime_steps_fillrate_cart() {
        let cart =
            load_cart_from_path("../bench/carts/fillrate.p8").expect("fillrate cart should parse");
        let mut runtime = LuaRuntime::new();
        runtime.load_cart(&cart).expect("runtime should load cart");
        runtime.step(0.0).expect("runtime should step");
        assert_eq!(runtime.frame_count(), 1);
        assert!(runtime.frame_buffer().iter().any(|&v| v != 0));
    }

    #[test]
    fn rust_lua_runtime_supports_buttons_flags_and_random() {
        let cart = load_cart_from_source(
            "inline:test",
            r#"pico-8 cartridge // http://www.pico-8.com
version 42
__lua__
function _init()
 fset(0,2,true)
end

function _draw()
 cls(0)
 if fget(0,2) then pset(0,0,7) end
 if btn(4) then pset(1,0,8) end
 if btnp(5) then pset(2,0,9) end
 if btn() == 48 then pset(3,0,10) end
 if btnp() == 32 then pset(4,0,11) end
 pset(5,0,flr(rnd(16)))
end
"#,
        )
        .expect("inline cart should parse");

        let mut runtime = LuaRuntime::new();
        runtime.load_cart(&cart).expect("runtime should load cart");
        runtime
            .step_with_input(
                InputState {
                    down: 1 << 5,
                    held: (1 << 4) | (1 << 5),
                },
                0.0,
            )
            .expect("runtime should step with input");

        let fb = runtime.frame_buffer();
        assert_eq!(fb[0], 7);
        assert_eq!(fb[1], 8);
        assert_eq!(fb[2], 9);
        assert_eq!(fb[3], 10);
        assert_eq!(fb[4], 11);
        assert!(fb[5] < 16);
    }
}
