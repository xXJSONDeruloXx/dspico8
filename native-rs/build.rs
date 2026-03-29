use std::path::PathBuf;

fn main() {
    let manifest_dir =
        PathBuf::from(std::env::var("CARGO_MANIFEST_DIR").expect("CARGO_MANIFEST_DIR must be set"));
    let lua_dir = manifest_dir.join("../third_party/lua");

    println!(
        "cargo:rerun-if-changed={}",
        lua_dir.join("onelua.c").display()
    );
    println!("cargo:rerun-if-changed={}", lua_dir.join("lua.h").display());
    println!(
        "cargo:rerun-if-changed={}",
        lua_dir.join("lauxlib.h").display()
    );
    println!(
        "cargo:rerun-if-changed={}",
        lua_dir.join("lualib.h").display()
    );

    cc::Build::new()
        .file(lua_dir.join("onelua.c"))
        .include(&lua_dir)
        .define("MAKE_LIB", None)
        .warnings(false)
        .compile("lua54_embedded");

    if std::env::var("CARGO_CFG_TARGET_OS").as_deref() == Ok("linux") {
        println!("cargo:rustc-link-lib=m");
        println!("cargo:rustc-link-lib=dl");
    }
}
