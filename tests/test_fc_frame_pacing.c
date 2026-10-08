#include "gb_frame_pacing.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    gb_frame_pacing_t pacing;gb_frame_pacing_init(&pacing);
    assert(gb_frame_pacing_next(0,50000)==GB_FRAME_PERIOD_US);
    assert(gb_frame_pacing_next(0,1000000)==1000000);
    assert(!gb_frame_pacing_draw_due(&pacing,50000,0));
    // 模拟实测的慢绘屏：绘制帧 55 ms，非绘制帧 9 ms。
    // CPU/APU 仍应按约 60 Hz 前进，而不是每次迟到都丢掉时间欠账。
    int64_t now=0,deadline=0;unsigned draws=0;
    for(int frame=0;frame<600;frame++) {
        bool draw=gb_frame_pacing_draw_due(&pacing,now,deadline);
        if(draw) gb_frame_pacing_mark_drawn(&pacing);
        unsigned cost=draw?55000:9000;draws+=draw;now+=cost;
        gb_frame_pacing_record(&pacing,cost);
        deadline=gb_frame_pacing_next(deadline,now);
        if(now<deadline)now=deadline;
    }
    assert(now>=10000000 && now<10300000);
    assert(draws>30 && draws<300);
    assert(pacing.draw_interval>=2 && pacing.draw_interval<=8);
    for(int i=0;i<2000;i++)gb_frame_pacing_record(&pacing,40000);
    assert(pacing.draw_interval==8);
    gb_frame_pacing_init(&pacing);
    gb_frame_pacing_mark_drawn(&pacing);
    pacing.frames=2; assert(gb_frame_pacing_draw(&pacing));
    // LCD 在第 2 帧仍忙，第 3 帧已空闲时不能再等到第 4 帧。
    pacing.frames=3; assert(gb_frame_pacing_draw(&pacing));
    gb_frame_pacing_mark_drawn(&pacing);
    pacing.frames=4; assert(!gb_frame_pacing_draw(&pacing));
    pacing.frames=5; assert(gb_frame_pacing_draw(&pacing));
    pacing.draw_interval=8;
    for(int i=0;i<120;i++)gb_frame_pacing_record(&pacing,13500);
    assert(pacing.draw_interval==7);
    puts("FC frame pacing: late LCD recovery and bounded adaptive rendering PASS");
}
