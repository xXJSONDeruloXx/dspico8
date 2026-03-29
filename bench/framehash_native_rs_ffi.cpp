#include "dsp_native_rs.h"

#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

namespace {
uint64_t fnv1a64(const uint8_t* data, size_t size) {
    uint64_t hash = 1469598103934665603ull;
    for (size_t i = 0; i < size; i++) {
        hash ^= static_cast<uint64_t>(data[i]);
        hash *= 1099511628211ull;
    }
    return hash;
}
} // namespace

int main(int argc, char** argv) {
    const std::string cartPath = argc > 1 ? argv[1] : "carts/fillrate.p8";
    const int frames = argc > 2 ? std::atoi(argv[2]) : 120;

    dsp_rs_runtime_handle* runtime = dsp_rs_runtime_new();
    if (runtime == nullptr) {
        std::cerr << "failed to create rust runtime handle\n";
        return 1;
    }

    if (!dsp_rs_runtime_load_cart_from_path(runtime, cartPath.c_str())) {
        std::cerr << dsp_rs_runtime_last_error(runtime) << "\n";
        dsp_rs_runtime_free(runtime);
        return 2;
    }

    for (int i = 0; i < frames; i++) {
        dsp_rs_input_state input{};
        if (!dsp_rs_runtime_step(runtime, input, i / 60.0)) {
            std::cerr << dsp_rs_runtime_last_error(runtime) << "\n";
            dsp_rs_runtime_free(runtime);
            return 3;
        }
    }

    const uint8_t* framebuffer = dsp_rs_runtime_frame_buffer(runtime);
    const uint64_t hash = fnv1a64(framebuffer, 128 * 128);
    std::cout << "runtime=native-rs-ffi-hash"
              << " cart=" << cartPath
              << " frames=" << frames
              << " fnv64=0x" << std::hex << std::nouppercase << hash << std::dec << "\n";

    dsp_rs_runtime_free(runtime);
    return 0;
}
