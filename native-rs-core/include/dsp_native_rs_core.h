#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void dsp_rs_core_global_reset(void);
const uint8_t* dsp_rs_core_global_frame_buffer(void);
size_t dsp_rs_core_global_frame_bytes(void);

void dsp_rs_core_global_pset(int32_t x, int32_t y, int32_t color);
void dsp_rs_core_global_rectfill(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t color);
void dsp_rs_core_global_rect(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t color);
void dsp_rs_core_global_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t color);
void dsp_rs_core_global_demo_frame(uint32_t ticks);

#ifdef __cplusplus
}
#endif
