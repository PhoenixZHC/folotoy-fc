#pragma once
#include <stdbool.h>
#include <stdint.h>
#define GB_FRAME_PERIOD_US 16667LL

typedef struct {
    uint8_t draw_interval;
    uint32_t frames;
    uint32_t window_frames;
    uint64_t window_work_us;
    uint32_t last_draw_frame;
    bool has_drawn;
} gb_frame_pacing_t;

void gb_frame_pacing_init(gb_frame_pacing_t *pacing);
bool gb_frame_pacing_draw(const gb_frame_pacing_t *pacing);
void gb_frame_pacing_mark_drawn(gb_frame_pacing_t *pacing);
bool gb_frame_pacing_draw_due(const gb_frame_pacing_t *pacing, int64_t now, int64_t deadline);
int64_t gb_frame_pacing_next(int64_t deadline, int64_t now);
void gb_frame_pacing_record(gb_frame_pacing_t *pacing, uint32_t work_us);
