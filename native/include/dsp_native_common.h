#pragma once

#include <cstdint>

namespace dsp::native {

constexpr int kScreenWidth = 128;
constexpr int kScreenHeight = 128;
constexpr int kSpriteSheetWidth = 128;
constexpr int kSpriteSheetHeight = 128;
constexpr int kMapWidth = 128;
constexpr int kMapHeight = 64;

struct InputState {
    uint8_t down = 0;
    uint8_t held = 0;
};

} // namespace dsp::native
