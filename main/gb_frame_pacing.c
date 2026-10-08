#include "gb_frame_pacing.h"

void gb_frame_pacing_init(gb_frame_pacing_t *pacing) {
    *pacing = (gb_frame_pacing_t){.draw_interval = 2};
}

bool gb_frame_pacing_draw(const gb_frame_pacing_t *pacing) {
    return !pacing->has_drawn || (uint32_t)(pacing->frames-pacing->last_draw_frame)>=pacing->draw_interval;
}

void gb_frame_pacing_mark_drawn(gb_frame_pacing_t *pacing) {
    pacing->last_draw_frame=pacing->frames;
    pacing->has_drawn=true;
}

bool gb_frame_pacing_draw_due(const gb_frame_pacing_t *pacing, int64_t now, int64_t deadline) {
    // 落后时先补 CPU/APU，避免追加一次同步 LCD 传输让音频再次断粮。
    return now-deadline <= GB_FRAME_PERIOD_US/2 && gb_frame_pacing_draw(pacing);
}

int64_t gb_frame_pacing_next(int64_t deadline,int64_t now) {
    deadline += GB_FRAME_PERIOD_US;
    // 保留短暂绘屏造成的时间欠账，允许后续不绘屏的帧补回进度。
    // 超过 250 ms 才重新同步，防止异常长停顿后无限追赶。
    return now-deadline > 250000 ? now : deadline;
}

void gb_frame_pacing_record(gb_frame_pacing_t *pacing, uint32_t work_us) {
    pacing->frames++;
    pacing->window_frames++;
    pacing->window_work_us += work_us;
    if (pacing->window_frames < 120) return;
    uint64_t average = pacing->window_work_us / pacing->window_frames;
    if (average > 15000 && pacing->draw_interval < 8) pacing->draw_interval++;
    else if (average < 14000 && pacing->draw_interval > 2) pacing->draw_interval--;
    pacing->window_frames = 0;
    pacing->window_work_us = 0;
}
