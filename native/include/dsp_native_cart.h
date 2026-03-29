#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace dsp::native {

constexpr int kScreenWidth = 128;
constexpr int kScreenHeight = 128;
constexpr int kSpriteSheetWidth = 128;
constexpr int kSpriteSheetHeight = 128;
constexpr int kMapWidth = 128;
constexpr int kMapHeight = 64;

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
