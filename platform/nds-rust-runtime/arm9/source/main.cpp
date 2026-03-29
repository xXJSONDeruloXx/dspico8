#include <fat.h>
#include <nds.h>

#include <array>
#include <cstdio>
#include <ctime>
#include <unistd.h>

#include "dsp_native_builtin_carts.h"
#include "dsp_native_rs.h"

namespace {
constexpr int kFrameBytes = 128 * 128;

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

void SetupVideo() {
    vramSetBankA(VRAM_A_MAIN_BG);
    vramSetBankC(VRAM_C_SUB_BG);
    videoSetMode(MODE_5_2D);
    videoSetModeSub(MODE_0_2D);
    bgInit(3, BgType_Bmp8, BgSize_B8_128x128, 0, 0);
    consoleDemoInit();
    lcdMainOnTop();
    bgSetScroll(3, -64, -32);
    bgSetScale(3, 0x100, 0x100);
    bgUpdate();

    for (int i = 0; i < 16; i++) {
        BG_PALETTE[i] = ToRgb555(kPicoPalette[i]);
    }
}

dsp_rs_input_state ScanPicoInput() {
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

    dsp_rs_input_state state {};
    state.down = convert(down);
    state.held = convert(held);
    return state;
}

bool QuitRequested() {
    const u32 held = keysHeld();
    return (held & (KEY_L | KEY_R)) == (KEY_L | KEY_R);
}

} // namespace

int main() {
    SetupVideo();
    consoleClear();
    iprintf("dspico8 rust runtime smoke\n");

    if (fatInitDefault()) {
        chdir(isDSiMode() ? "sd:/" : "fat:/");
        iprintf("fat ok\n");
    } else {
        iprintf("fat unavailable\n");
    }

    dsp_rs_runtime_handle* runtime = dsp_rs_runtime_new();
    if (runtime == nullptr) {
        iprintf("runtime alloc failed\n");
        while (true) {
            swiWaitForVBlank();
        }
    }

    if (!dsp_rs_runtime_load_cart_from_source(runtime, "builtin:fillrate", dsp::native::kBuiltinBenchmarkCart)) {
        iprintf("load failed:\n%s\n", dsp_rs_runtime_last_error(runtime));
        while (true) {
            swiWaitForVBlank();
        }
    }

    iprintf("runtime ok\n");
    iprintf("a/b = p8 buttons\n");
    iprintf("hold l+r to quit\n\n");

    clock_t lastTicks = clock();
    uint64_t lastFrameCount = 0;
    double fps = 0.0;

    while (!QuitRequested()) {
        const dsp_rs_input_state input = ScanPicoInput();
        const double nowSeconds = static_cast<double>(clock()) / static_cast<double>(CLOCKS_PER_SEC);
        if (!dsp_rs_runtime_step(runtime, input, nowSeconds)) {
            consoleClear();
            iprintf("runtime error:\n%s\n", dsp_rs_runtime_last_error(runtime));
            while (true) {
                swiWaitForVBlank();
            }
        }

        dmaCopyWords(0, dsp_rs_runtime_frame_buffer(runtime), bgGetGfxPtr(3), kFrameBytes);

        const clock_t nowTicks = clock();
        const double elapsed = static_cast<double>(nowTicks - lastTicks) / static_cast<double>(CLOCKS_PER_SEC);
        if (elapsed >= 0.5) {
            const uint64_t frameCount = dsp_rs_runtime_frame_count(runtime);
            const uint64_t frames = frameCount - lastFrameCount;
            fps = frames / elapsed;
            lastTicks = nowTicks;
            lastFrameCount = frameCount;

            iprintf("\x1b[5;0H");
            iprintf("frames: %-8llu\n", static_cast<unsigned long long>(frameCount));
            iprintf("fps:    %2.2f   \n", fps);
            iprintf("mode: rust runtime\n");
        }

        swiWaitForVBlank();
    }

    dsp_rs_runtime_free(runtime);
    return 0;
}
