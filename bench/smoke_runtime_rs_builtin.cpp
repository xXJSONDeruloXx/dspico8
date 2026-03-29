#include "dsp_native_builtin_carts.h"
#include "dsp_native_runtime_rs.h"

#include <cstdint>
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

int main() {
    dsp::native::RuntimeRs runtime;
    std::string error;
    if (!runtime.LoadCartFromSource("builtin:fillrate", dsp::native::kBuiltinBenchmarkCart, error)) {
        std::cerr << error << "\n";
        return 1;
    }

    for (int i = 0; i < 120; i++) {
        if (!runtime.Step({}, i / 60.0, error)) {
            std::cerr << error << "\n";
            return 2;
        }
    }

    const uint8_t* framebuffer = runtime.FrameBuffer();
    if (framebuffer == nullptr) {
        std::cerr << "missing framebuffer\n";
        return 3;
    }

    const uint64_t hash = fnv1a64(framebuffer, 128 * 128);
    std::cout << "runtime=native-rs-cpp-source-smoke"
              << " frames=" << runtime.FrameCount()
              << " fnv64=0x" << std::hex << std::nouppercase << hash << std::dec << "\n";
    return 0;
}
