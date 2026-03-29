use dsp_native_rs::load_cart_from_path;

fn main() {
    let mut args = std::env::args().skip(1).collect::<Vec<_>>();
    if args.is_empty() {
        eprintln!("usage: cargo run --manifest-path native-rs/Cargo.toml --bin cart_probe -- <cart1> [cart2 ...]");
        std::process::exit(2);
    }

    for path in args.drain(..) {
        match load_cart_from_path(&path) {
            Ok(cart) => {
                let nonzero_gfx = cart.gfx.iter().filter(|&&v| v != 0).count();
                let nonzero_map = cart.map.iter().filter(|&&v| v != 0).count();
                let nonzero_flags = cart.flags.iter().filter(|&&v| v != 0).count();
                println!(
                    "cart={} lua_bytes={} nonzero_gfx={} nonzero_map={} nonzero_flags={}",
                    path,
                    cart.lua.len(),
                    nonzero_gfx,
                    nonzero_map,
                    nonzero_flags
                );
            }
            Err(error) => {
                eprintln!("cart={} error={}", path, error);
                std::process::exit(1);
            }
        }
    }
}
