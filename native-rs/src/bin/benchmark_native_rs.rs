use dsp_native_rs::{load_cart_from_path, LuaRuntime};

use std::time::Instant;

fn main() {
    let args = std::env::args().collect::<Vec<_>>();
    let cart_path = args
        .get(1)
        .map(String::as_str)
        .unwrap_or("bench/carts/fillrate.p8");
    let warmup_frames = args
        .get(2)
        .and_then(|s| s.parse::<usize>().ok())
        .unwrap_or(120);
    let measured_frames = args
        .get(3)
        .and_then(|s| s.parse::<usize>().ok())
        .unwrap_or(600);

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

    for i in 0..warmup_frames {
        if let Err(error) = runtime.step(i as f64 / 60.0) {
            eprintln!("{}", error);
            std::process::exit(3);
        }
    }

    let start = Instant::now();
    for i in 0..measured_frames {
        if let Err(error) = runtime.step((warmup_frames + i) as f64 / 60.0) {
            eprintln!("{}", error);
            std::process::exit(4);
        }
    }
    let elapsed = start.elapsed();

    let micros = elapsed.as_micros() as f64;
    let micros_per_frame = micros / measured_frames as f64;
    let fps = 1_000_000.0 / micros_per_frame;

    println!(
        "runtime=native-rs cart={} frames={} us_per_frame={:.2} fps_equivalent={:.2}",
        cart_path, measured_frames, micros_per_frame, fps
    );
}
