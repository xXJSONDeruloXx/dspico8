#pragma once

#include "dsp_native_common.h"

#include <array>
#include <cstdint>
#include <string>

namespace dsp::native {

struct Cart {
    std::string name;
    std::string lua;
    std::array<uint8_t, kSpriteSheetWidth * kSpriteSheetHeight> gfx {};
    std::array<uint8_t, kMapWidth * kMapHeight> map {};
    std::array<uint8_t, 256> flags {};
};

bool LoadCartFromP8File(const std::string& path, Cart& outCart, std::string& error);
bool LoadCartFromP8String(const std::string& name, const std::string& source, Cart& outCart, std::string& error);

} // namespace dsp::native
