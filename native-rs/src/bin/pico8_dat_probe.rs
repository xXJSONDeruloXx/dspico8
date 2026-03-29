use dsp_native_rs::scan_interesting_chunks;

fn main() {
    let mut args = std::env::args().skip(1);
    let path = match args.next() {
        Some(path) => path,
        None => {
            eprintln!("usage: cargo run --manifest-path native-rs/Cargo.toml --bin pico8_dat_probe -- <pico8.dat> [limit]");
            std::process::exit(2);
        }
    };
    let limit = args
        .next()
        .and_then(|v| v.parse::<usize>().ok())
        .unwrap_or(40);

    let bytes = match std::fs::read(&path) {
        Ok(bytes) => bytes,
        Err(error) => {
            eprintln!("failed to read {}: {}", path, error);
            std::process::exit(1);
        }
    };

    let hits = scan_interesting_chunks(&bytes);
    println!(
        "file={} hits={} showing={}",
        path,
        hits.len(),
        limit.min(hits.len())
    );
    for hit in hits.iter().take(limit) {
        if hit.tokens.is_empty() {
            println!("offset=0x{:x} tag={} tokens=[]", hit.offset, hit.tag);
        } else {
            println!(
                "offset=0x{:x} tag={} tokens={}",
                hit.offset,
                hit.tag,
                hit.tokens.join(", ")
            );
        }
    }
}
