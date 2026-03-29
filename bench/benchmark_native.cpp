#include "dsp_native_cart.h"
#include "dsp_native_runtime.h"

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    const std::string cartPath = argc > 1 ? argv[1] : "carts/fillrate.p8";
    const int warmupFrames = argc > 2 ? std::atoi(argv[2]) : 120;
    const int measuredFrames = argc > 3 ? std::atoi(argv[3]) : 600;

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

    for (int i = 0; i < warmupFrames; i++) {
        if (!runtime.Step({}, i / 60.0, error)) {
            std::cerr << error << "\n";
            return 3;
        }
    }

    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < measuredFrames; i++) {
        if (!runtime.Step({}, (warmupFrames + i) / 60.0, error)) {
            std::cerr << error << "\n";
            return 4;
        }
    }
    const auto end = std::chrono::steady_clock::now();

    const auto micros = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    const double microsPerFrame = static_cast<double>(micros) / measuredFrames;
    const double fps = 1000000.0 / microsPerFrame;

    std::cout << std::fixed << std::setprecision(2)
              << "runtime=native"
              << " cart=" << cartPath
              << " frames=" << measuredFrames
              << " us_per_frame=" << microsPerFrame
              << " fps_equivalent=" << fps << "\n";
    return 0;
}
