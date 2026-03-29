#include "dsp_native_cart.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>

namespace dsp::native {
namespace {

enum class Section {
    None,
    Lua,
    Gfx,
    Map,
    Gff,
};

int HexDigitToInt(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

bool StartsWith(const std::string& line, const char* prefix) {
    return line.rfind(prefix, 0) == 0;
}

void ParseGfxLine(const std::string& line, int row, Cart& cart) {
    if (row < 0 || row >= kSpriteSheetHeight) return;

    const int width = std::min<int>(static_cast<int>(line.size()), kSpriteSheetWidth);
    for (int x = 0; x < width; x++) {
        const int v = HexDigitToInt(line[x]);
        if (v >= 0) {
            cart.gfx[row * kSpriteSheetWidth + x] = static_cast<uint8_t>(v);
        }
    }
}

void ParseMapLine(const std::string& line, int row, Cart& cart) {
    if (row < 0 || row >= kMapHeight) return;

    const int byteCount = std::min<int>(static_cast<int>(line.size() / 2), kMapWidth);
    for (int x = 0; x < byteCount; x++) {
        const int hi = HexDigitToInt(line[x * 2]);
        const int lo = HexDigitToInt(line[x * 2 + 1]);
        if (hi >= 0 && lo >= 0) {
            cart.map[row * kMapWidth + x] = static_cast<uint8_t>((hi << 4) | lo);
        }
    }
}

void ParseFlagLine(const std::string& line, int row, Cart& cart) {
    const int start = row * 128;
    if (start >= static_cast<int>(cart.flags.size())) return;

    const int byteCount = std::min<int>(static_cast<int>(line.size() / 2), 128);
    for (int i = 0; i < byteCount; i++) {
        const int hi = HexDigitToInt(line[i * 2]);
        const int lo = HexDigitToInt(line[i * 2 + 1]);
        if (hi >= 0 && lo >= 0) {
            const int idx = start + i;
            if (idx < static_cast<int>(cart.flags.size())) {
                cart.flags[idx] = static_cast<uint8_t>((hi << 4) | lo);
            }
        }
    }
}

} // namespace

bool LoadCartFromP8String(const std::string& name, const std::string& source, Cart& outCart, std::string& error) {
    std::istringstream stream(source);
    std::string line;
    Section section = Section::None;
    int gfxRow = 0;
    int mapRow = 0;
    int gffRow = 0;

    Cart cart;
    cart.name = name;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (StartsWith(line, "__lua__")) {
            section = Section::Lua;
            continue;
        }
        if (StartsWith(line, "__gfx__")) {
            section = Section::Gfx;
            continue;
        }
        if (StartsWith(line, "__map__")) {
            section = Section::Map;
            continue;
        }
        if (StartsWith(line, "__gff__")) {
            section = Section::Gff;
            continue;
        }
        if (StartsWith(line, "__")) {
            section = Section::None;
            continue;
        }

        switch (section) {
            case Section::Lua:
                cart.lua += line;
                cart.lua.push_back('\n');
                break;
            case Section::Gfx:
                ParseGfxLine(line, gfxRow++, cart);
                break;
            case Section::Map:
                ParseMapLine(line, mapRow++, cart);
                break;
            case Section::Gff:
                ParseFlagLine(line, gffRow++, cart);
                break;
            case Section::None:
                break;
        }
    }

    if (cart.lua.empty()) {
        error = "cart has no __lua__ section";
        return false;
    }

    outCart = cart;
    return true;
}

bool LoadCartFromP8File(const std::string& path, Cart& outCart, std::string& error) {
    std::ifstream file(path);
    if (!file.is_open()) {
        error = "failed to open cart: " + path;
        return false;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return LoadCartFromP8String(path, buffer.str(), outCart, error);
}

} // namespace dsp::native
