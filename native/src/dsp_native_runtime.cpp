#include "dsp_native_runtime.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>

namespace dsp::native {
namespace {

constexpr char kRuntimeRegistryKey = 0;

Runtime* GetRuntime(lua_State* L) {
    lua_pushlightuserdata(L, const_cast<char*>(&kRuntimeRegistryKey));
    lua_gettable(L, LUA_REGISTRYINDEX);
    auto* runtime = static_cast<Runtime*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return runtime;
}

int Clamp(int value, int minValue, int maxValue) {
    return std::max(minValue, std::min(value, maxValue));
}

int ApiCls(lua_State* L) {
    auto* runtime = GetRuntime(L);
    const int color = lua_isnoneornil(L, 1) ? 0 : static_cast<int>(luaL_checkinteger(L, 1));
    std::fill(runtime->frameBuffer_.begin(), runtime->frameBuffer_.end(), static_cast<uint8_t>(color & 0x0f));
    return 0;
}

int ApiColor(lua_State* L) {
    auto* runtime = GetRuntime(L);
    const int previous = runtime->color_;
    if (!lua_isnoneornil(L, 1)) {
        runtime->color_ = static_cast<uint8_t>(luaL_checkinteger(L, 1) & 0x0f);
    }
    lua_pushinteger(L, previous);
    return 1;
}

int ApiPset(lua_State* L) {
    auto* runtime = GetRuntime(L);
    const int x = static_cast<int>(luaL_checkinteger(L, 1));
    const int y = static_cast<int>(luaL_checkinteger(L, 2));
    const int color = lua_isnoneornil(L, 3) ? runtime->color_ : static_cast<int>(luaL_checkinteger(L, 3));
    runtime->PutPixel(x - runtime->cameraX_, y - runtime->cameraY_, static_cast<uint8_t>(color & 0x0f));
    return 0;
}

int ApiPget(lua_State* L) {
    auto* runtime = GetRuntime(L);
    const int x = static_cast<int>(luaL_checkinteger(L, 1));
    const int y = static_cast<int>(luaL_checkinteger(L, 2));
    lua_pushinteger(L, runtime->GetPixel(x - runtime->cameraX_, y - runtime->cameraY_));
    return 1;
}

int ApiRectfill(lua_State* L) {
    auto* runtime = GetRuntime(L);
    int x0 = static_cast<int>(luaL_checkinteger(L, 1)) - runtime->cameraX_;
    int y0 = static_cast<int>(luaL_checkinteger(L, 2)) - runtime->cameraY_;
    int x1 = static_cast<int>(luaL_checkinteger(L, 3)) - runtime->cameraX_;
    int y1 = static_cast<int>(luaL_checkinteger(L, 4)) - runtime->cameraY_;
    const uint8_t color = static_cast<uint8_t>((lua_isnoneornil(L, 5) ? runtime->color_ : luaL_checkinteger(L, 5)) & 0x0f);

    if (x0 > x1) std::swap(x0, x1);
    if (y0 > y1) std::swap(y0, y1);

    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            runtime->PutPixel(x, y, color);
        }
    }
    return 0;
}

int ApiRect(lua_State* L) {
    auto* runtime = GetRuntime(L);
    int x0 = static_cast<int>(luaL_checkinteger(L, 1)) - runtime->cameraX_;
    int y0 = static_cast<int>(luaL_checkinteger(L, 2)) - runtime->cameraY_;
    int x1 = static_cast<int>(luaL_checkinteger(L, 3)) - runtime->cameraX_;
    int y1 = static_cast<int>(luaL_checkinteger(L, 4)) - runtime->cameraY_;
    const uint8_t color = static_cast<uint8_t>((lua_isnoneornil(L, 5) ? runtime->color_ : luaL_checkinteger(L, 5)) & 0x0f);

    if (x0 > x1) std::swap(x0, x1);
    if (y0 > y1) std::swap(y0, y1);

    for (int x = x0; x <= x1; x++) {
        runtime->PutPixel(x, y0, color);
        runtime->PutPixel(x, y1, color);
    }
    for (int y = y0; y <= y1; y++) {
        runtime->PutPixel(x0, y, color);
        runtime->PutPixel(x1, y, color);
    }
    return 0;
}

int ApiLine(lua_State* L) {
    auto* runtime = GetRuntime(L);
    int x0 = static_cast<int>(luaL_checkinteger(L, 1)) - runtime->cameraX_;
    int y0 = static_cast<int>(luaL_checkinteger(L, 2)) - runtime->cameraY_;
    int x1 = static_cast<int>(luaL_checkinteger(L, 3)) - runtime->cameraX_;
    int y1 = static_cast<int>(luaL_checkinteger(L, 4)) - runtime->cameraY_;
    const uint8_t color = static_cast<uint8_t>((lua_isnoneornil(L, 5) ? runtime->color_ : luaL_checkinteger(L, 5)) & 0x0f);

    const int dx = std::abs(x1 - x0);
    const int sx = x0 < x1 ? 1 : -1;
    const int dy = -std::abs(y1 - y0);
    const int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    for (;;) {
        runtime->PutPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        const int e2 = err * 2;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
    return 0;
}

int ApiSget(lua_State* L) {
    auto* runtime = GetRuntime(L);
    lua_pushinteger(L, runtime->GetSpritePixel(static_cast<int>(luaL_checkinteger(L, 1)), static_cast<int>(luaL_checkinteger(L, 2))));
    return 1;
}

int ApiSset(lua_State* L) {
    auto* runtime = GetRuntime(L);
    runtime->SetSpritePixel(
        static_cast<int>(luaL_checkinteger(L, 1)),
        static_cast<int>(luaL_checkinteger(L, 2)),
        static_cast<uint8_t>(luaL_checkinteger(L, 3) & 0x0f));
    return 0;
}

int ApiSpr(lua_State* L) {
    auto* runtime = GetRuntime(L);
    const int n = static_cast<int>(luaL_checkinteger(L, 1));
    const int dx = static_cast<int>(luaL_checkinteger(L, 2)) - runtime->cameraX_;
    const int dy = static_cast<int>(luaL_checkinteger(L, 3)) - runtime->cameraY_;
    const int w = lua_isnoneornil(L, 4) ? 1 : std::max(1, static_cast<int>(luaL_checknumber(L, 4)));
    const int h = lua_isnoneornil(L, 5) ? 1 : std::max(1, static_cast<int>(luaL_checknumber(L, 5)));
    const bool flipX = !lua_isnoneornil(L, 6) && lua_toboolean(L, 6);
    const bool flipY = !lua_isnoneornil(L, 7) && lua_toboolean(L, 7);

    const int spriteX = (n % 16) * 8;
    const int spriteY = (n / 16) * 8;
    for (int sy = 0; sy < h * 8; sy++) {
        for (int sx = 0; sx < w * 8; sx++) {
            const int srcX = spriteX + (flipX ? (w * 8 - 1 - sx) : sx);
            const int srcY = spriteY + (flipY ? (h * 8 - 1 - sy) : sy);
            const uint8_t color = runtime->GetSpritePixel(srcX, srcY);
            if (color != 0) {
                runtime->PutPixel(dx + sx, dy + sy, color);
            }
        }
    }
    return 0;
}

int ApiMap(lua_State* L) {
    auto* runtime = GetRuntime(L);
    const int celX = static_cast<int>(luaL_checkinteger(L, 1));
    const int celY = static_cast<int>(luaL_checkinteger(L, 2));
    const int sx = static_cast<int>(luaL_checkinteger(L, 3));
    const int sy = static_cast<int>(luaL_checkinteger(L, 4));
    const int celW = static_cast<int>(luaL_checkinteger(L, 5));
    const int celH = static_cast<int>(luaL_checkinteger(L, 6));

    for (int y = 0; y < celH; y++) {
        for (int x = 0; x < celW; x++) {
            const uint8_t tile = runtime->GetMapCell(celX + x, celY + y);
            lua_settop(L, 0);
            lua_pushinteger(L, tile);
            lua_pushinteger(L, sx + x * 8);
            lua_pushinteger(L, sy + y * 8);
            ApiSpr(L);
        }
    }
    lua_settop(L, 0);
    return 0;
}

int ApiMget(lua_State* L) {
    auto* runtime = GetRuntime(L);
    lua_pushinteger(L, runtime->GetMapCell(static_cast<int>(luaL_checkinteger(L, 1)), static_cast<int>(luaL_checkinteger(L, 2))));
    return 1;
}

int ApiMset(lua_State* L) {
    auto* runtime = GetRuntime(L);
    runtime->SetMapCell(
        static_cast<int>(luaL_checkinteger(L, 1)),
        static_cast<int>(luaL_checkinteger(L, 2)),
        static_cast<uint8_t>(luaL_checkinteger(L, 3) & 0xff));
    return 0;
}

int ApiFget(lua_State* L) {
    auto* runtime = GetRuntime(L);
    const int sprite = static_cast<int>(luaL_checkinteger(L, 1)) & 0xff;
    if (lua_isnoneornil(L, 2)) {
        lua_pushinteger(L, runtime->flags_[sprite]);
    } else {
        const int bit = static_cast<int>(luaL_checkinteger(L, 2));
        lua_pushboolean(L, (runtime->flags_[sprite] >> bit) & 0x1);
    }
    return 1;
}

int ApiFset(lua_State* L) {
    auto* runtime = GetRuntime(L);
    const int sprite = static_cast<int>(luaL_checkinteger(L, 1)) & 0xff;
    if (lua_gettop(L) >= 3 && lua_isboolean(L, 3)) {
        const int bit = static_cast<int>(luaL_checkinteger(L, 2)) & 7;
        const bool value = lua_toboolean(L, 3);
        if (value) runtime->flags_[sprite] |= static_cast<uint8_t>(1u << bit);
        else runtime->flags_[sprite] &= static_cast<uint8_t>(~(1u << bit));
    } else {
        runtime->flags_[sprite] = static_cast<uint8_t>(luaL_checkinteger(L, 2) & 0xff);
    }
    return 0;
}

int ApiBtn(lua_State* L) {
    auto* runtime = GetRuntime(L);
    if (lua_isnoneornil(L, 1)) {
        lua_pushinteger(L, runtime->input_.held);
        return 1;
    }

    const int bit = static_cast<int>(luaL_checkinteger(L, 1));
    lua_pushboolean(L, ((runtime->input_.held >> bit) & 0x1) != 0);
    return 1;
}

int ApiBtnp(lua_State* L) {
    auto* runtime = GetRuntime(L);
    if (lua_isnoneornil(L, 1)) {
        lua_pushinteger(L, runtime->input_.down);
        return 1;
    }

    const int bit = static_cast<int>(luaL_checkinteger(L, 1));
    lua_pushboolean(L, ((runtime->input_.down >> bit) & 0x1) != 0);
    return 1;
}

int ApiCamera(lua_State* L) {
    auto* runtime = GetRuntime(L);
    const int prevX = runtime->cameraX_;
    const int prevY = runtime->cameraY_;
    if (lua_gettop(L) >= 2) {
        runtime->cameraX_ = static_cast<int>(luaL_checkinteger(L, 1));
        runtime->cameraY_ = static_cast<int>(luaL_checkinteger(L, 2));
    } else {
        runtime->cameraX_ = 0;
        runtime->cameraY_ = 0;
    }
    lua_pushinteger(L, prevX);
    lua_pushinteger(L, prevY);
    return 2;
}

int ApiClip(lua_State* L) {
    auto* runtime = GetRuntime(L);
    const int prevX = runtime->clipX0_;
    const int prevY = runtime->clipY0_;
    const int prevW = runtime->clipX1_ - runtime->clipX0_;
    const int prevH = runtime->clipY1_ - runtime->clipY0_;

    if (lua_gettop(L) >= 4) {
        const int x = static_cast<int>(luaL_checkinteger(L, 1));
        const int y = static_cast<int>(luaL_checkinteger(L, 2));
        const int w = static_cast<int>(luaL_checkinteger(L, 3));
        const int h = static_cast<int>(luaL_checkinteger(L, 4));
        runtime->clipX0_ = Clamp(x, 0, kScreenWidth);
        runtime->clipY0_ = Clamp(y, 0, kScreenHeight);
        runtime->clipX1_ = Clamp(x + w, 0, kScreenWidth);
        runtime->clipY1_ = Clamp(y + h, 0, kScreenHeight);
    } else {
        runtime->clipX0_ = 0;
        runtime->clipY0_ = 0;
        runtime->clipX1_ = kScreenWidth;
        runtime->clipY1_ = kScreenHeight;
    }

    lua_pushinteger(L, prevX);
    lua_pushinteger(L, prevY);
    lua_pushinteger(L, prevW);
    lua_pushinteger(L, prevH);
    return 4;
}

int ApiFlip(lua_State* L) {
    (void)L;
    return 0;
}

int ApiTime(lua_State* L) {
    auto* runtime = GetRuntime(L);
    lua_pushnumber(L, runtime->timeSeconds_);
    return 1;
}

int ApiPrint(lua_State* L) {
    const char* text = lua_tostring(L, 1);
    const int len = text ? static_cast<int>(std::strlen(text)) : 0;
    lua_pushinteger(L, len * 4);
    return 1;
}

int ApiRnd(lua_State* L) {
    auto* runtime = GetRuntime(L);
    std::uniform_real_distribution<double> unitDist(0.0, 1.0);
    if (lua_isnoneornil(L, 1)) {
        lua_pushnumber(L, unitDist(runtime->rng_));
        return 1;
    }

    const double bound = luaL_checknumber(L, 1);
    lua_pushnumber(L, unitDist(runtime->rng_) * bound);
    return 1;
}

int ApiPal(lua_State* L) {
    (void)L;
    return 0;
}

int ApiPalt(lua_State* L) {
    (void)L;
    return 0;
}

} // namespace

Runtime::Runtime() : rng_(0xD5C1C08u) {
    ResetDrawState();
}

Runtime::~Runtime() {
    ResetLua();
}

void Runtime::ResetLua() {
    if (lua_ != nullptr) {
        lua_close(lua_);
        lua_ = nullptr;
    }
}

void Runtime::ResetDrawState() {
    color_ = 6;
    cameraX_ = 0;
    cameraY_ = 0;
    clipX0_ = 0;
    clipY0_ = 0;
    clipX1_ = kScreenWidth;
    clipY1_ = kScreenHeight;
    std::fill(frameBuffer_.begin(), frameBuffer_.end(), 0);
}

void Runtime::RegisterApi() {
    lua_pushlightuserdata(lua_, const_cast<char*>(&kRuntimeRegistryKey));
    lua_pushlightuserdata(lua_, this);
    lua_settable(lua_, LUA_REGISTRYINDEX);

    lua_register(lua_, "cls", ApiCls);
    lua_register(lua_, "color", ApiColor);
    lua_register(lua_, "pset", ApiPset);
    lua_register(lua_, "pget", ApiPget);
    lua_register(lua_, "rectfill", ApiRectfill);
    lua_register(lua_, "rect", ApiRect);
    lua_register(lua_, "line", ApiLine);
    lua_register(lua_, "sget", ApiSget);
    lua_register(lua_, "sset", ApiSset);
    lua_register(lua_, "spr", ApiSpr);
    lua_register(lua_, "map", ApiMap);
    lua_register(lua_, "mget", ApiMget);
    lua_register(lua_, "mset", ApiMset);
    lua_register(lua_, "fget", ApiFget);
    lua_register(lua_, "fset", ApiFset);
    lua_register(lua_, "btn", ApiBtn);
    lua_register(lua_, "btnp", ApiBtnp);
    lua_register(lua_, "camera", ApiCamera);
    lua_register(lua_, "clip", ApiClip);
    lua_register(lua_, "flip", ApiFlip);
    lua_register(lua_, "time", ApiTime);
    lua_register(lua_, "t", ApiTime);
    lua_register(lua_, "print", ApiPrint);
    lua_register(lua_, "rnd", ApiRnd);
    lua_register(lua_, "pal", ApiPal);
    lua_register(lua_, "palt", ApiPalt);
}

bool Runtime::CallIfExists(const char* functionName, std::string& error) {
    lua_getglobal(lua_, functionName);
    if (!lua_isfunction(lua_, -1)) {
        lua_pop(lua_, 1);
        return true;
    }

    if (lua_pcall(lua_, 0, 0, 0) != LUA_OK) {
        error = lua_tostring(lua_, -1);
        lua_pop(lua_, 1);
        return false;
    }
    return true;
}

bool Runtime::LoadCartFromPath(const std::string& path, std::string& error) {
    Cart cart;
    if (!LoadCartFromP8File(path, cart, error)) {
        return false;
    }
    return LoadCart(cart, error);
}

bool Runtime::LoadCartFromSource(const std::string& name, const std::string& source, std::string& error) {
    Cart cart;
    if (!LoadCartFromP8String(name, source, cart, error)) {
        return false;
    }
    return LoadCart(cart, error);
}

bool Runtime::LoadCart(const Cart& cart, std::string& error) {
    ResetLua();
    ResetDrawState();
    spriteSheet_ = cart.gfx;
    map_ = cart.map;
    flags_ = cart.flags;
    frameCount_ = 0;
    timeSeconds_ = 0.0;

    lua_ = luaL_newstate();
    if (lua_ == nullptr) {
        error = "failed to create lua state";
        return false;
    }

    luaL_requiref(lua_, LUA_GNAME, luaopen_base, 1);
    lua_pop(lua_, 1);
    luaL_requiref(lua_, LUA_TABLIBNAME, luaopen_table, 1);
    lua_pop(lua_, 1);
    luaL_requiref(lua_, LUA_STRLIBNAME, luaopen_string, 1);
    lua_pop(lua_, 1);
    luaL_requiref(lua_, LUA_MATHLIBNAME, luaopen_math, 1);
    lua_pop(lua_, 1);
    luaL_requiref(lua_, LUA_UTF8LIBNAME, luaopen_utf8, 1);
    lua_pop(lua_, 1);
    luaL_requiref(lua_, LUA_COLIBNAME, luaopen_coroutine, 1);
    lua_pop(lua_, 1);
    RegisterApi();

    static const char* kHelpers = R"LUA(
flr = math.floor
ceil = math.ceil
min = math.min
max = math.max
abs = math.abs
mid = function(a,b,c)
  local x, y, z = a, b, c
  if x > y then x, y = y, x end
  if y > z then y, z = z, y end
  if x > y then x, y = y, x end
  return y
end
add = function(t, v)
  t[#t + 1] = v
  return v
end
count = function(t)
  return #t
end
del = function(t, v)
  for i = 1, #t do
    if t[i] == v then
      table.remove(t, i)
      return v
    end
  end
  return nil
end
foreach = function(t, fn)
  for i = 1, #t do fn(t[i]) end
end
all = function(t)
  local i = 0
  return function()
    i = i + 1
    return t[i]
  end
end
)LUA";

    if (luaL_dostring(lua_, kHelpers) != LUA_OK) {
        error = lua_tostring(lua_, -1);
        lua_pop(lua_, 1);
        return false;
    }

    if (luaL_loadstring(lua_, cart.lua.c_str()) != LUA_OK) {
        error = lua_tostring(lua_, -1);
        lua_pop(lua_, 1);
        return false;
    }

    if (lua_pcall(lua_, 0, 0, 0) != LUA_OK) {
        error = lua_tostring(lua_, -1);
        lua_pop(lua_, 1);
        return false;
    }

    return CallIfExists("_init", error);
}

bool Runtime::Step(const InputState& input, double timeSeconds, std::string& error) {
    input_ = input;
    timeSeconds_ = timeSeconds;
    ++frameCount_;

    lua_getglobal(lua_, "_update60");
    if (!lua_isfunction(lua_, -1)) {
        lua_pop(lua_, 1);
        lua_getglobal(lua_, "_update");
    }
    if (lua_isfunction(lua_, -1)) {
        if (lua_pcall(lua_, 0, 0, 0) != LUA_OK) {
            error = lua_tostring(lua_, -1);
            lua_pop(lua_, 1);
            return false;
        }
    } else {
        lua_pop(lua_, 1);
    }

    return CallIfExists("_draw", error);
}

void Runtime::PutPixel(int x, int y, uint8_t color) {
    if (x < clipX0_ || x >= clipX1_ || y < clipY0_ || y >= clipY1_) return;
    PutPixelRaw(x, y, color);
}

void Runtime::PutPixelRaw(int x, int y, uint8_t color) {
    if (x < 0 || x >= kScreenWidth || y < 0 || y >= kScreenHeight) return;
    frameBuffer_[y * kScreenWidth + x] = static_cast<uint8_t>(color & 0x0f);
}

uint8_t Runtime::GetPixel(int x, int y) const {
    if (x < 0 || x >= kScreenWidth || y < 0 || y >= kScreenHeight) return 0;
    return frameBuffer_[y * kScreenWidth + x];
}

uint8_t Runtime::GetSpritePixel(int x, int y) const {
    if (x < 0 || x >= kSpriteSheetWidth || y < 0 || y >= kSpriteSheetHeight) return 0;
    return spriteSheet_[y * kSpriteSheetWidth + x];
}

void Runtime::SetSpritePixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= kSpriteSheetWidth || y < 0 || y >= kSpriteSheetHeight) return;
    spriteSheet_[y * kSpriteSheetWidth + x] = static_cast<uint8_t>(color & 0x0f);
}

uint8_t Runtime::GetMapCell(int x, int y) const {
    if (x < 0 || x >= kMapWidth || y < 0 || y >= kMapHeight) return 0;
    return map_[y * kMapWidth + x];
}

void Runtime::SetMapCell(int x, int y, uint8_t value) {
    if (x < 0 || x >= kMapWidth || y < 0 || y >= kMapHeight) return;
    map_[y * kMapWidth + x] = value;
}

} // namespace dsp::native
