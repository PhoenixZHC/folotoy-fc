// 固定输入比较优化前后的实际 ROM 图像序列与音频；不代替真机帧率验收。
#include "gb_port.h"
#include "nes/nes.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc,char **argv) {
    if(argc!=3) return 2;
    FILE *file=fopen(argv[1],"rb"); if(!file) return 3;
    fseek(file,0,SEEK_END);long bytes=ftell(file);rewind(file);
    uint8_t *rom=malloc(bytes); if(!rom || fread(rom,1,bytes,file)!=(size_t)bytes)return 4;
    fclose(file);
    gb_rom_source_t source={.mapped=rom,.size=(uint32_t)bytes};
    gb_session_t *s=NULL;if(!gb_port_create(&source,&s))return 5;
    FILE *audio=fopen(argv[2],"wb");if(!audio)return 6;
    uint32_t hash=2166136261u;unsigned changes=0,previous=0;size_t samples=0;
    for(int i=0;i<3600;i++) {
        gb_port_set_keys(s,(gb_port_keys_t){.start=i>=120 && i<124,
            .a=i>=180,.right=i>=600 && i%240<60});
        if(!gb_port_step_frame(s,i%3==0))return 7;
        if(i%3==0) {
            uint32_t frame_hash=2166136261u;
            const uint8_t *pixels=gb_port_framebuffer(s);
            for(int y=0;y<240;y++)for(int x=0;x<256;x++)frame_hash=(frame_hash^(pixels[y*272+x+8]&63))*16777619u;
            changes+=frame_hash!=previous;previous=frame_hash;
            hash=(hash^frame_hash)*16777619u;
        }
        int16_t pcm[267];size_t n=gb_port_audio_frame(s,pcm,267);
        if(fwrite(pcm,sizeof(*pcm),n,audio)!=n)return 8;
        samples+=n;
    }
    fclose(audio);
    printf("COMPARE_RESULT {\"video_hash\":%u,\"changed_frames\":%u,\"samples\":%zu}\n",hash,changes,samples);
    gb_port_destroy(s);free(rom);return 0;
}
