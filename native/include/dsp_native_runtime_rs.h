#pragma once

#include "dsp_native_cart.h"
#include "dsp_native_rs.h"

#include <cstdint>
#include <string>

namespace dsp::native {

class RuntimeRs {
public:
    RuntimeRs();
    ~RuntimeRs();

    RuntimeRs(const RuntimeRs&) = delete;
    RuntimeRs& operator=(const RuntimeRs&) = delete;

    bool LoadCart(const Cart& cart, std::string& error);
    bool LoadCartFromPath(const std::string& path, std::string& error);
    bool LoadCartFromSource(const std::string& name, const std::string& source, std::string& error);
    bool Step(const InputState& input, double timeSeconds, std::string& error);

    const uint8_t* FrameBuffer() const;
    uint64_t FrameCount() const;

private:
    dsp_rs_runtime_handle* handle_ = nullptr;
};

} // namespace dsp::native
