#![cfg_attr(not(feature = "std"), no_std)]

extern crate alloc;

#[cfg(not(feature = "std"))]
mod no_std_support {
    use core::alloc::{GlobalAlloc, Layout};
    use core::ffi::c_void;
    use core::ptr;

    const MIN_ALIGN: usize = if core::mem::align_of::<u64>() > core::mem::size_of::<usize>() {
        core::mem::align_of::<u64>()
    } else {
        core::mem::size_of::<usize>()
    };

    unsafe extern "C" {
        fn malloc(size: usize) -> *mut c_void;
        fn free(ptr: *mut c_void);
        fn realloc(ptr: *mut c_void, size: usize) -> *mut c_void;
        fn memalign(boundary: usize, size: usize) -> *mut c_void;
    }

    struct LibcAllocator;

    unsafe impl GlobalAlloc for LibcAllocator {
        unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
            if layout.align() <= MIN_ALIGN {
                malloc(layout.size()) as *mut u8
            } else {
                memalign(layout.align(), layout.size()) as *mut u8
            }
        }

        unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
            free(ptr as *mut c_void);
        }

        unsafe fn realloc(&self, ptr: *mut u8, layout: Layout, new_size: usize) -> *mut u8 {
            if layout.align() <= MIN_ALIGN {
                realloc(ptr as *mut c_void, new_size) as *mut u8
            } else {
                let new_ptr =
                    self.alloc(Layout::from_size_align_unchecked(new_size, layout.align()));
                if !new_ptr.is_null() {
                    ptr::copy_nonoverlapping(ptr, new_ptr, core::cmp::min(layout.size(), new_size));
                    self.dealloc(ptr, layout);
                }
                new_ptr
            }
        }
    }

    #[global_allocator]
    static GLOBAL_ALLOCATOR: LibcAllocator = LibcAllocator;
}

pub mod cart;
pub mod ffi;
pub mod lua_runtime;
pub mod pico8_dat;

#[cfg(feature = "std")]
pub use cart::load_cart_from_path;
pub use cart::{load_cart_from_bytes, load_cart_from_source, Cart};
pub use dsp_native_rs_core::{
    RuntimeCore, MAP_HEIGHT, MAP_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH, SPRITE_SHEET_HEIGHT,
    SPRITE_SHEET_WIDTH,
};
pub use lua_runtime::{InputState, LuaRuntime};
pub use pico8_dat::{scan_interesting_chunks, DatHit};
