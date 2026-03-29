#include "dsp_native_runtime_rs.h"

namespace dsp::native {
namespace {

bool SetErrorFromHandle(dsp_rs_runtime_handle* handle, std::string& error) {
    const char* lastError = dsp_rs_runtime_last_error(handle);
    error = lastError != nullptr ? lastError : "rust runtime ffi error";
    return false;
}

} // namespace

RuntimeRs::RuntimeRs() : handle_(dsp_rs_runtime_new()) {
}

RuntimeRs::~RuntimeRs() {
    dsp_rs_runtime_free(handle_);
    handle_ = nullptr;
}

bool RuntimeRs::LoadCart(const Cart& cart, std::string& error) {
    if (handle_ == nullptr) {
        error = "failed to create rust runtime handle";
        return false;
    }

    dsp_rs_cart_desc desc{};
    desc.name = cart.name.c_str();
    desc.lua = cart.lua.c_str();
    desc.gfx = cart.gfx.data();
    desc.map = cart.map.data();
    desc.flags = cart.flags.data();

    if (!dsp_rs_runtime_load_cart_from_parts(handle_, &desc)) {
        return SetErrorFromHandle(handle_, error);
    }

    error.clear();
    return true;
}

bool RuntimeRs::LoadCartFromPath(const std::string& path, std::string& error) {
    if (handle_ == nullptr) {
        error = "failed to create rust runtime handle";
        return false;
    }

    if (!dsp_rs_runtime_load_cart_from_path(handle_, path.c_str())) {
        return SetErrorFromHandle(handle_, error);
    }

    error.clear();
    return true;
}

bool RuntimeRs::LoadCartFromSource(const std::string& name, const std::string& source, std::string& error) {
    if (handle_ == nullptr) {
        error = "failed to create rust runtime handle";
        return false;
    }

    if (!dsp_rs_runtime_load_cart_from_source(handle_, name.c_str(), source.c_str())) {
        return SetErrorFromHandle(handle_, error);
    }

    error.clear();
    return true;
}

bool RuntimeRs::Step(const InputState& input, double timeSeconds, std::string& error) {
    if (handle_ == nullptr) {
        error = "failed to create rust runtime handle";
        return false;
    }

    dsp_rs_input_state rsInput {};
    rsInput.down = input.down;
    rsInput.held = input.held;

    if (!dsp_rs_runtime_step(handle_, rsInput, timeSeconds)) {
        return SetErrorFromHandle(handle_, error);
    }

    error.clear();
    return true;
}

const uint8_t* RuntimeRs::FrameBuffer() const {
    return handle_ != nullptr ? dsp_rs_runtime_frame_buffer(handle_) : nullptr;
}

uint64_t RuntimeRs::FrameCount() const {
    return handle_ != nullptr ? dsp_rs_runtime_frame_count(handle_) : 0;
}

} // namespace dsp::native
