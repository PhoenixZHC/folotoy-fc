// 主机运行验收工具：使用实际核心，记录帧变化和音频；不声称完整游戏兼容。
#include "gb_port.h"
#include "fc_rom.h"
#include "nes/nes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned hash_frame(uint8_t *const *frame) {
    unsigned h=2166136261u;
    for(int y=0;y<240;y++) for(int x=0;x<256;x++) h=(h^(frame[y][x+8]&63))*16777619u;
    return h;
}
int main(int argc,char **argv) {
    if(argc<3) return 2;
    FILE *f=fopen(argv[1],"rb"); if(!f) return 3;
    fseek(f,0,SEEK_END); long size=ftell(f); rewind(f);
    if(size<16 || size>4194304) { fclose(f); return 4; }
    uint8_t *data=malloc(size);
    if(!data || fread(data,1,size,f)!=(size_t)size) { fclose(f); free(data); return 5; }
    fclose(f);
    gb_rom_source_t source={.mapped=data,.size=(uint32_t)size};
    gb_session_t *session=NULL;
    if(!gb_port_create(&source,&session)) { free(data); puts("ROM_REJECTED"); return 6; }
    int frames=atoi(argv[2]); if(frames<1 || frames>36000) return 7;
    int draw_every=argc>4?atoi(argv[4]):1;
    if(draw_every<1 || draw_every>3) return 7;
    unsigned previous=0,changes=0; size_t audible=0,samples=0;
    for(int i=0;i<frames;i++) {
        // 前两秒观测标题；随后间歇 Start、方向和 A，属于固定输入烟测。
        gb_port_keys_t keys={.start=i>=120 && i%120<4,
                            .right=i>=180 && i%240<120,.a=i>=180 && i%30<10};
        gb_port_set_keys(session,keys);
        if(!gb_port_step_frame(session,i%draw_every==0)) return 8;
        unsigned hash=hash_frame(gb_port_framebuffer(session));
        if(i && hash!=previous) changes++;
        previous=hash;
        int16_t pcm[267]; size_t n=gb_port_audio_frame(session,pcm,267); samples+=n;
        for(size_t j=0;j<n;j++) audible+=pcm[j]!=0;
    }
    uint8_t *const *frame=gb_port_framebuffer(session);
    bool colors[64]={0}; unsigned distinct=0;
    for(int y=0;y<240;y++) for(int x=0;x<256;x++) colors[frame[y][x+8]&63]=true;
    for(int i=0;i<64;i++) distinct+=colors[i];
    if(argc>3) {
        FILE *image=fopen(argv[3],"wb"); if(!image) return 9;
        fprintf(image,"P6\n256 240\n255\n");
        const uint16_t *pal=fc_port_palette(session);
        for(int y=0;y<240;y++) for(int x=0;x<256;x++) {
            uint16_t c=pal[frame[y][x+8]&63];
            uint8_t rgb[]={((c>>11)&31)*255/31,((c>>5)&63)*255/63,(c&31)*255/31};
            fwrite(rgb,1,3,image);
        }
        fclose(image);
    }
    printf("ROM_RESULT {\"frames\":%d,\"changed_frames\":%u,\"colors\":%u,\"samples\":%zu,\"audible_samples\":%zu}\n",frames,changes,distinct,samples,audible);
    gb_port_destroy(session); free(data); return 0;
}
