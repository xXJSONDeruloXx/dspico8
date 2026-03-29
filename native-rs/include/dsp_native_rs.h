#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dsp_rs_runtime_handle dsp_rs_runtime_handle;

typedef struct dsp_rs_input_state {
    uint8_t down;
    uint8_t held;
} dsp_rs_input_state;

typedef struct dsp_rs_cart_desc {
    const char* name;
    const char* lua;
    const uint8_t* gfx;
    const uint8_t* map;
    const uint8_t* flags;
} dsp_rs_cart_desc;

dsp_rs_runtime_handle* dsp_rs_runtime_new(void);
void dsp_rs_runtime_free(dsp_rs_runtime_handle* handle);

bool dsp_rs_runtime_load_cart_from_path(dsp_rs_runtime_handle* handle, const char* path);
bool dsp_rs_runtime_load_cart_from_parts(dsp_rs_runtime_handle* handle, const dsp_rs_cart_desc* cart);
bool dsp_rs_runtime_step(dsp_rs_runtime_handle* handle, dsp_rs_input_state input, double time_seconds);

const uint8_t* dsp_rs_runtime_frame_buffer(const dsp_rs_runtime_handle* handle);
uint64_t dsp_rs_runtime_frame_count(const dsp_rs_runtime_handle* handle);
const char* dsp_rs_runtime_last_error(const dsp_rs_runtime_handle* handle);

#ifdef __cplusplus
}
#endif
