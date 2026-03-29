use crate::{load_cart_from_path, InputState, LuaRuntime};

use std::ffi::{CStr, CString};
use std::os::raw::c_char;
use std::ptr;

#[repr(C)]
pub struct dsp_rs_input_state {
    pub down: u8,
    pub held: u8,
}

#[repr(C)]
pub struct dsp_rs_cart_desc {
    pub name: *const c_char,
    pub lua: *const c_char,
    pub gfx: *const u8,
    pub map: *const u8,
    pub flags: *const u8,
}

#[repr(C)]
pub struct dsp_rs_runtime_handle {
    runtime: LuaRuntime,
    last_error: CString,
}

impl dsp_rs_runtime_handle {
    fn new() -> Self {
        Self {
            runtime: LuaRuntime::new(),
            last_error: CString::new("").expect("empty CString should be valid"),
        }
    }

    fn set_error(&mut self, message: String) {
        let sanitized = message.replace('\0', " ");
        self.last_error =
            CString::new(sanitized).unwrap_or_else(|_| CString::new("rust ffi error").unwrap());
    }

    fn clear_error(&mut self) {
        self.last_error = CString::new("").expect("empty CString should be valid");
    }
}

fn read_c_string(ptr: *const c_char, field_name: &str) -> Result<String, String> {
    if ptr.is_null() {
        return Err(format!("missing {field_name} string"));
    }

    Ok(unsafe { CStr::from_ptr(ptr) }
        .to_string_lossy()
        .into_owned())
}

fn copy_fixed<const N: usize>(src: *const u8) -> [u8; N] {
    let mut out = [0u8; N];
    if !src.is_null() {
        unsafe {
            ptr::copy_nonoverlapping(src, out.as_mut_ptr(), N);
        }
    }
    out
}

#[no_mangle]
pub extern "C" fn dsp_rs_runtime_new() -> *mut dsp_rs_runtime_handle {
    Box::into_raw(Box::new(dsp_rs_runtime_handle::new()))
}

#[no_mangle]
pub extern "C" fn dsp_rs_runtime_free(handle: *mut dsp_rs_runtime_handle) {
    if handle.is_null() {
        return;
    }
    unsafe {
        drop(Box::from_raw(handle));
    }
}

#[no_mangle]
pub extern "C" fn dsp_rs_runtime_load_cart_from_path(
    handle: *mut dsp_rs_runtime_handle,
    path: *const c_char,
) -> bool {
    if handle.is_null() || path.is_null() {
        return false;
    }

    let handle = unsafe { &mut *handle };
    let path_str = match unsafe { CStr::from_ptr(path) }.to_str() {
        Ok(path_str) => path_str,
        Err(error) => {
            handle.set_error(format!("invalid cart path: {error}"));
            return false;
        }
    };

    match load_cart_from_path(path_str) {
        Ok(cart) => match handle.runtime.load_cart(&cart) {
            Ok(()) => {
                handle.clear_error();
                true
            }
            Err(error) => {
                handle.set_error(error);
                false
            }
        },
        Err(error) => {
            handle.set_error(error);
            false
        }
    }
}

#[no_mangle]
pub extern "C" fn dsp_rs_runtime_load_cart_from_parts(
    handle: *mut dsp_rs_runtime_handle,
    cart: *const dsp_rs_cart_desc,
) -> bool {
    if handle.is_null() || cart.is_null() {
        return false;
    }

    let handle = unsafe { &mut *handle };
    let cart = unsafe { &*cart };

    let name = match if cart.name.is_null() {
        Ok(String::from("ffi-cart"))
    } else {
        read_c_string(cart.name, "cart name")
    } {
        Ok(name) => name,
        Err(error) => {
            handle.set_error(error);
            return false;
        }
    };

    let lua = match read_c_string(cart.lua, "cart lua") {
        Ok(lua) => lua,
        Err(error) => {
            handle.set_error(error);
            return false;
        }
    };

    let rust_cart = crate::Cart {
        name,
        lua,
        gfx: copy_fixed(cart.gfx),
        map: copy_fixed(cart.map),
        flags: copy_fixed(cart.flags),
    };

    match handle.runtime.load_cart(&rust_cart) {
        Ok(()) => {
            handle.clear_error();
            true
        }
        Err(error) => {
            handle.set_error(error);
            false
        }
    }
}

#[no_mangle]
pub extern "C" fn dsp_rs_runtime_step(
    handle: *mut dsp_rs_runtime_handle,
    input: dsp_rs_input_state,
    time_seconds: f64,
) -> bool {
    if handle.is_null() {
        return false;
    }

    let handle = unsafe { &mut *handle };
    match handle.runtime.step_with_input(
        InputState {
            down: input.down,
            held: input.held,
        },
        time_seconds,
    ) {
        Ok(()) => {
            handle.clear_error();
            true
        }
        Err(error) => {
            handle.set_error(error);
            false
        }
    }
}

#[no_mangle]
pub extern "C" fn dsp_rs_runtime_frame_buffer(handle: *const dsp_rs_runtime_handle) -> *const u8 {
    if handle.is_null() {
        return ptr::null();
    }

    let handle = unsafe { &*handle };
    handle.runtime.frame_buffer().as_ptr()
}

#[no_mangle]
pub extern "C" fn dsp_rs_runtime_frame_count(handle: *const dsp_rs_runtime_handle) -> u64 {
    if handle.is_null() {
        return 0;
    }

    let handle = unsafe { &*handle };
    handle.runtime.frame_count()
}

#[no_mangle]
pub extern "C" fn dsp_rs_runtime_last_error(handle: *const dsp_rs_runtime_handle) -> *const c_char {
    if handle.is_null() {
        return ptr::null();
    }

    let handle = unsafe { &*handle };
    handle.last_error.as_ptr()
}
