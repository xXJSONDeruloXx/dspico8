#pragma once

#include "dsp_native_cart.h"

#include <array>
#include <cstdint>
#include <random>
#include <string>

struct lua_State;

namespace dsp::native {

class Runtime {
public:
    Runtime();
    ~Runtime();

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;

    bool LoadCart(const Cart& cart, std::string& error);
    bool Step(const InputState& input, double timeSeconds, std::string& error);

    const uint8_t* FrameBuffer() const { return frameBuffer_.data(); }
    const uint8_t* SpriteSheet() const { return spriteSheet_.data(); }
    uint64_t FrameCount() const { return frameCount_; }

public:
    // Exposed for the C Lua binding layer implemented in dsp_native_runtime.cpp.
    void ResetLua();
    void ResetDrawState();
    void RegisterApi();
    bool CallIfExists(const char* functionName, std::string& error);

    void PutPixel(int x, int y, uint8_t color);
    void PutPixelRaw(int x, int y, uint8_t color);
    uint8_t GetPixel(int x, int y) const;
    uint8_t GetSpritePixel(int x, int y) const;
    void SetSpritePixel(int x, int y, uint8_t color);
    uint8_t GetMapCell(int x, int y) const;
    void SetMapCell(int x, int y, uint8_t value);

    lua_State* lua_ = nullptr;
    std::array<uint8_t, kScreenWidth * kScreenHeight> frameBuffer_ {};
    std::array<uint8_t, kSpriteSheetWidth * kSpriteSheetHeight> spriteSheet_ {};
    std::array<uint8_t, kMapWidth * kMapHeight> map_ {};
    std::array<uint8_t, 256> flags_ {};

    InputState input_ {};
    uint8_t color_ = 6;
    int cameraX_ = 0;
    int cameraY_ = 0;
    int clipX0_ = 0;
    int clipY0_ = 0;
    int clipX1_ = kScreenWidth;
    int clipY1_ = kScreenHeight;
    double timeSeconds_ = 0.0;
    uint64_t frameCount_ = 0;
    std::mt19937 rng_;
};

} // namespace dsp::native
