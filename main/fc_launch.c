#include "fc_launch.h"
#include <string.h>

static void rect(uint8_t *frame,int x,int y,int w,int h,unsigned shade) {
    for(int dy=y;dy<y+h;dy++) for(int dx=x;dx<x+w;dx++) {
        if(dx<0 || dx>=GB_WIDTH || dy<0 || dy>=GB_HEIGHT) continue;
        unsigned shift=(dx%4)*2;
        uint8_t *pixel=&frame[dy*(GB_WIDTH/4)+dx/4];
        *pixel=(*pixel & ~(3u<<shift)) | (shade<<shift);
    }
}
void fc_launch_frame(uint8_t frame[GB_FRAME_BYTES],uint32_t elapsed_ms) {
    memset(frame,0,GB_FRAME_BYTES);
    // 卡带滑入插槽，机身遮住下沿；最后亮灯并保持短暂停顿。
    unsigned progress=elapsed_ms>640?640:elapsed_ms;
    int top=12+(int)(progress*32/640);
    rect(frame,51,top,58,42,3);rect(frame,54,top+3,52,36,1);
    rect(frame,59,top+7,42,23,0);
    static const uint8_t letters[2][7]={{31,16,16,30,16,16,16},{15,16,16,16,16,16,15}};
    for(int letter=0;letter<2;letter++) for(int y=0;y<7;y++) for(int x=0;x<5;x++)
        if(letters[letter][y] & (16>>x)) rect(frame,65+letter*18+x*2,top+11+y*2,2,2,3);
    rect(frame,24,73,112,48,3);rect(frame,27,76,106,42,0);
    rect(frame,31,78,98,5,3);rect(frame,34,79,92,2,2);
    rect(frame,33,89,29,19,2);rect(frame,70,89,56,19,1);
    rect(frame,39,93,5,12,3);rect(frame,36,96,11,5,3);
    rect(frame,99,96,7,7,3);rect(frame,112,96,7,7,3);
    rect(frame,33,112,6,3,elapsed_ms>=640?3:1);
    rect(frame,50,112,36,3,3);rect(frame,92,112,34,3,2);
    rect(frame,44,127,72,3,1);
    rect(frame,44,127,(int)((elapsed_ms>FC_LAUNCH_MS?FC_LAUNCH_MS:elapsed_ms)*72/FC_LAUNCH_MS),3,3);
}
