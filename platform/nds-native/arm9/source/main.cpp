#include <fat.h>
#include <nds.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <dirent.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "dsp_native_builtin_carts.h"
#include "dsp_native_cart.h"
#include "dsp_native_runtime.h"

namespace {
using dsp::native::Cart;
using dsp::native::InputState;
using dsp::native::Runtime;

constexpr int kFrameBytes = dsp::native::kScreenWidth * dsp::native::kScreenHeight;

struct Rgb {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

constexpr std::array<Rgb, 16> kPicoPalette = {{
    {0x00, 0x00, 0x00}, {0x1d, 0x2b, 0x53}, {0x7e, 0x25, 0x53}, {0x00, 0x87, 0x51},
    {0xab, 0x52, 0x36}, {0x5f, 0x57, 0x4f}, {0xc2, 0xc3, 0xc7}, {0xff, 0xf1, 0xe8},
    {0xff, 0x00, 0x4d}, {0xff, 0xa3, 0x00}, {0xff, 0xec, 0x27}, {0x00, 0xe4, 0x36},
    {0x29, 0xad, 0xff}, {0x83, 0x76, 0x9c}, {0xff, 0x77, 0xa8}, {0xff, 0xcc, 0xaa},
}};

uint16_t ToRgb555(const Rgb& color) {
    return RGB15(color.r >> 3, color.g >> 3, color.b >> 3);
}

bool HasP8Extension(const std::string& path) {
    return path.size() >= 3 && path.substr(path.size() - 3) == ".p8";
}

std::vector<std::string> ListP8Carts(const std::string& directory) {
    std::vector<std::string> carts;
    DIR* dir = opendir(directory.c_str());
    if (dir == nullptr) return carts;

    while (auto* ent = readdir(dir)) {
        if (ent->d_name[0] == '.') continue;
        std::string path = directory + "/" + ent->d_name;
        if (HasP8Extension(path)) {
            carts.push_back(path);
        }
    }

    closedir(dir);
    std::sort(carts.begin(), carts.end());
    return carts;
}

void SetupVideo() {
    vramSetBankA(VRAM_A_MAIN_BG);
    vramSetBankB(VRAM_B_MAIN_SPRITE);
    vramSetBankC(VRAM_C_SUB_BG);
    vramSetBankD(VRAM_D_SUB_SPRITE);
    videoSetMode(MODE_5_2D);
    videoSetModeSub(MODE_5_2D);
    bgInit(3, BgType_Bmp8, BgSize_B8_128x128, 0, 0);
    consoleInit(nullptr, 0, BgType_Text4bpp, BgSize_T_256x256, 0, 1, false, true);
    lcdMainOnTop();
    bgSetScroll(3, -64, -32);
    bgSetScale(3, 0x100, 0x100);
    bgUpdate();

    for (int i = 0; i < 16; i++) {
        BG_PALETTE[i] = ToRgb555(kPicoPalette[i]);
    }
}

InputState ScanPicoInput() {
    scanKeys();
    const u32 down = keysDown();
    const u32 held = keysHeld();

    auto convert = [](u32 keys) {
        uint8_t result = 0;
        if (keys & KEY_LEFT) result |= 1u << 0;
        if (keys & KEY_RIGHT) result |= 1u << 1;
        if (keys & KEY_UP) result |= 1u << 2;
        if (keys & KEY_DOWN) result |= 1u << 3;
        if (keys & KEY_B) result |= 1u << 4;
        if (keys & KEY_A) result |= 1u << 5;
        if (keys & KEY_START) result |= 1u << 6;
        return result;
    };

    return InputState{convert(down), convert(held)};
}

bool QuitRequested() {
    const u32 held = keysHeld();
    return (held & (KEY_L | KEY_R)) == (KEY_L | KEY_R);
}

bool LoadStartupCart(Cart& cart, std::string& loadedName, std::string& error) {
    auto carts = ListP8Carts("/p8carts");
    if (!carts.empty()) {
        loadedName = carts.front();
        return dsp::native::LoadCartFromP8File(loadedName, cart, error);
    }

    loadedName = "builtin:fillrate";
    return dsp::native::LoadCartFromP8String(loadedName, dsp::native::kBuiltinBenchmarkCart, cart, error);
}

} // namespace

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    SetupVideo();
    consoleClear();
    iprintf("dspico8 native runtime\n");
    iprintf("initializing fat...\n");

    if (fatInitDefault()) {
        chdir(isDSiMode() ? "sd:/" : "fat:/");
        iprintf("fat ok\n");
    } else {
        iprintf("fat unavailable, using builtin cart\n");
    }

    Cart cart;
    std::string error;
    std::string loadedName;
    iprintf("loading cart...\n");
    if (!LoadStartupCart(cart, loadedName, error)) {
        iprintf("cart load failed:\n%s\n", error.c_str());
        while (true) {
            swiWaitForVBlank();
        }
    }

    iprintf("cart parsed ok\n");
    Runtime runtime;
    iprintf("loading lua...\n");
    if (!runtime.LoadCart(cart, error)) {
        iprintf("runtime load failed:\n%s\n", error.c_str());
        while (true) {
            swiWaitForVBlank();
        }
    }

    iprintf("runtime ok\n");
    iprintf("cart: %s\n", loadedName.c_str());
    iprintf("a/b = p8 buttons\n");
    iprintf("start = btn(6)\n");
    iprintf("hold l+r to quit\n\n");

    clock_t lastTicks = clock();
    uint64_t lastFrameCount = 0;
    double fps = 0.0;

    while (!QuitRequested()) {
        const InputState input = ScanPicoInput();
        const double nowSeconds = static_cast<double>(clock()) / static_cast<double>(CLOCKS_PER_SEC);
        if (!runtime.Step(input, nowSeconds, error)) {
            consoleClear();
            iprintf("runtime error:\n%s\n", error.c_str());
            while (true) {
                swiWaitForVBlank();
            }
        }

        dmaCopyWords(0, runtime.FrameBuffer(), bgGetGfxPtr(3), kFrameBytes);

        const clock_t nowTicks = clock();
        const double elapsed = static_cast<double>(nowTicks - lastTicks) / static_cast<double>(CLOCKS_PER_SEC);
        if (elapsed >= 0.5) {
            const uint64_t frames = runtime.FrameCount() - lastFrameCount;
            fps = frames / elapsed;
            lastTicks = nowTicks;
            lastFrameCount = runtime.FrameCount();

            iprintf("\x1b[7;0H");
            iprintf("frames: %-8llu\n", static_cast<unsigned long long>(runtime.FrameCount()));
            iprintf("fps:    %2.2f   \n", fps);
            iprintf("mode: native 8bpp\n");
        }

        swiWaitForVBlank();
    }

    return 0;
}
