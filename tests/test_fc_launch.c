#include "fc_launch.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv) {
    struct { uint8_t before[16], frame[GB_FRAME_BYTES], after[16]; } guarded;
    uint8_t first[GB_FRAME_BYTES],last[GB_FRAME_BYTES];
    memset(&guarded,0xa5,sizeof(guarded));
    fc_launch_frame(first,0);fc_launch_frame(last,FC_LAUNCH_MS);
    assert(memcmp(first,last,sizeof(first))!=0);
    uint32_t times[]={0,40,320,640,1000,UINT32_MAX};
    for(unsigned t=0;t<sizeof(times)/sizeof(times[0]);t++) {
        fc_launch_frame(guarded.frame,times[t]);
        for(int i=0;i<16;i++) assert(guarded.before[i]==0xa5 && guarded.after[i]==0xa5);
    }
    assert(!memcmp(last,guarded.frame,sizeof(last)));
    if(argc==2) {
        FILE *f=fopen(argv[1],"wb");assert(f);fprintf(f,"P6\n480 144\n255\n");
        uint8_t frames[3][GB_FRAME_BYTES];
        fc_launch_frame(frames[0],0);fc_launch_frame(frames[1],320);fc_launch_frame(frames[2],1000);
        for(int y=0;y<144;y++) for(int k=0;k<3;k++) for(int x=0;x<160;x++) {
            unsigned shade=(frames[k][y*40+x/4]>>((x%4)*2))&3;
            uint8_t value=255-shade*85,rgb[]={value,value,value};fwrite(rgb,1,3,f);
        }
        assert(fclose(f)==0);
    }
    puts("FC launch: bounded drawing, animation change, end-state clamp PASS");
}
