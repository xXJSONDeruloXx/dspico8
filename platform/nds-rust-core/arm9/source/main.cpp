#include <nds.h>

#include <array>
#include <cstdio>

#include "dsp_native_rs_core.h"

namespace {
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
} // namespace

int main() {
    SetupVideo();
    consoleClear();
    iprintf("dspico8 rust core smoke\n");
    iprintf("l+r to quit\n");

    dsp_rs_core_global_reset();

    uint32_t ticks = 0;
    while (true) {
        scanKeys();
        const u32 held = keysHeld();
        if ((held & (KEY_L | KEY_R)) == (KEY_L | KEY_R)) {
            break;
        }

        dsp_rs_core_global_demo_frame(ticks++);
        dmaCopyWords(0, dsp_rs_core_global_frame_buffer(), bgGetGfxPtr(3), dsp_rs_core_global_frame_bytes());

        if ((ticks & 31u) == 0) {
            iprintf("\x1b[2;0Hframes: %-8lu", static_cast<unsigned long>(ticks));
        }

        swiWaitForVBlank();
    }

    return 0;
}
