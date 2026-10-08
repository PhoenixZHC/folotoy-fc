#pragma once
#include <stdint.h>
#include "gb_pixels.h"
#define FC_LAUNCH_MS 1000u
void fc_launch_frame(uint8_t frame[GB_FRAME_BYTES], uint32_t elapsed_ms);
