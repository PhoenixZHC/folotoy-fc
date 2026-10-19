#include <assert.h>
#include <stdio.h>
#include "fc_video.h"

static uint8_t pixels[240][272];
static uint8_t *rows[240];
int main(void) {
    uint16_t palette[64], output[242];
    for (int i=0;i<64;i++) palette[i]=(uint16_t)(0x8000+i);
    for (int y=0;y<240;y++) {
        rows[y]=pixels[y];
        for (int x=0;x<272;x++) pixels[y][x]=(uint8_t)((x*7+y*13)&255);
    }
    // 两个坐标方向分开编码，检查整幅旋转的轴、方向及边界；不只检查对称图案。
    for(int axis=0;axis<2;axis++) {
        for(int y=0;y<240;y++)for(int x=0;x<256;x++)
            pixels[y][x+8]=(uint8_t)((axis?y:x)|0xc0);
        for(int mode=0;mode<FC_VIDEO_COUNT;mode++) {
            int height=fc_video_height(mode),top=fc_video_top(mode);
            assert(top>=0 && top+height<=320);
            for(int y=0;y<height;y++) {
                output[0]=0x1234;output[241]=0x5678;
                fc_video_row(rows,palette,y,mode,output+1);
                assert(output[0]==0x1234 && output[241]==0x5678);
                for(int x=0;x<240;x++) {
                    int sx=mode==2?255-y:mode?y:x*256/240;
                    int sy=mode==2?x:mode?239-x:y*240/225;
                    assert(output[x+1]==palette[(axis?sy:sx)&63]);
                }
            }
        }
    }
    assert(fc_video_height(true)==256 && fc_video_top(true)==32);
    assert(fc_video_height(false)==225 && fc_video_top(false)==40);
    puts("portrait and landscape mapping PASS");
}
