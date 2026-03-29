#include "../source/Audio.h"
#include "../source/PicoRam.h"
#include "../source/vm.h"
#include "../test/stubhost.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    const std::string cartPath = argc > 1 ? argv[1] : "carts/fillrate.p8";
    const int warmupFrames = argc > 2 ? std::atoi(argv[2]) : 120;
    const int measuredFrames = argc > 3 ? std::atoi(argv[3]) : 600;

    StubHost host;
    host.setUpPaletteColors();

    PicoRam memory;
    memory.Reset();
    Audio audio(&memory);
    Vm vm(&host, &memory, nullptr, nullptr, &audio);

    if (!vm.LoadCart(cartPath, false)) {
        std::cerr << "failed to load cart: " << cartPath << "\n";
        return 1;
    }

    vm.vm_run();

    std::string error;
    for (int i = 0; i < warmupFrames; i++) {
        if (!vm.Step()) {
            std::cerr << "step failed during warmup\n";
            return 2;
        }
    }

    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < measuredFrames; i++) {
        if (!vm.Step()) {
            std::cerr << "step failed during benchmark\n";
            return 3;
        }
    }
    const auto end = std::chrono::steady_clock::now();

    const auto micros = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    const double microsPerFrame = static_cast<double>(micros) / measuredFrames;
    const double fps = 1000000.0 / microsPerFrame;

    std::cout << std::fixed << std::setprecision(2)
              << "runtime=fake08"
              << " cart=" << cartPath
              << " frames=" << measuredFrames
              << " us_per_frame=" << microsPerFrame
              << " fps_equivalent=" << fps << "\n";
    std::fflush(stdout);
    std::_Exit(0);
}
