use dsp_native_rs::{load_cart_from_path, LuaRuntime};

fn fnv1a64(data: &[u8]) -> u64 {
    let mut hash = 1469598103934665603u64;
    for &byte in data {
        hash ^= byte as u64;
        hash = hash.wrapping_mul(1099511628211u64);
    }
    hash
}

fn main() {
    let args = std::env::args().collect::<Vec<_>>();
    let cart_path = args.get(1).map(String::as_str).unwrap_or("bench/carts/fillrate.p8");
    let frames = args.get(2).and_then(|s| s.parse::<usize>().ok()).unwrap_or(120);

    let cart = match load_cart_from_path(cart_path) {
        Ok(cart) => cart,
        Err(error) => {
            eprintln!("{}", error);
            std::process::exit(1);
        }
    };

    let mut runtime = LuaRuntime::new();
    if let Err(error) = runtime.load_cart(&cart) {
        eprintln!("{}", error);
        std::process::exit(2);
    }

    for i in 0..frames {
        if let Err(error) = runtime.step(i as f64 / 60.0) {
            eprintln!("{}", error);
            std::process::exit(3);
        }
    }

    let hash = fnv1a64(runtime.frame_buffer());
    println!(
        "runtime=native-rs-hash cart={} frames={} fnv64=0x{:x}",
        cart_path, frames, hash
    );
}
