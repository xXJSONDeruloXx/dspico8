#include "dsp_native_cart.h"
#include "dsp_native_runtime.h"

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

    dsp::native::Cart cart;
    std::string error;
    if (!dsp::native::LoadCartFromP8File(cartPath, cart, error)) {
        std::cerr << error << "\n";
        return 1;
    }

    dsp::native::Runtime runtime;
    if (!runtime.LoadCart(cart, error)) {
        std::cerr << error << "\n";
        return 2;
    }

    for (int i = 0; i < frames; i++) {
        if (!runtime.Step({}, i / 60.0, error)) {
            std::cerr << error << "\n";
            return 3;
        }
    }

    const uint64_t hash = fnv1a64(runtime.FrameBuffer(), dsp::native::kScreenWidth * dsp::native::kScreenHeight);
    std::cout << "runtime=native-hash"
              << " cart=" << cartPath
              << " frames=" << frames
              << " fnv64=0x" << std::hex << std::nouppercase << hash << std::dec << "\n";
    return 0;
}
