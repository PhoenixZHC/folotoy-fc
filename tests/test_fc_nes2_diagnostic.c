#include "gb_port.h"
#include "nes/nes.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void step(gb_session_t *s, gb_port_keys_t keys, int frames) {
    gb_port_set_keys(s,keys);
    for(int i=0;i<frames;i++) assert(gb_port_step_frame(s,true));
}
int main(int argc,char **argv) {
    assert(argc==2);
    FILE *f=fopen(argv[1],"rb"); assert(f);
    fseek(f,0,SEEK_END); size_t size=(size_t)ftell(f); rewind(f);
    uint8_t *rom=malloc(size); assert(rom && fread(rom,1,size,f)==size); fclose(f);
    gb_rom_source_t source={.mapped=rom,.size=size};
    gb_session_t *s=NULL;
    assert(gb_port_create(&source,&s));
    step(s,(gb_port_keys_t){0},120);
    uint8_t *nt=ppu_getnametable(0);
    assert(!memcmp(nt+21*32+4,"NEW SAVE",8));
    assert(nt[19*32+15]=='0');
    assert(nt[4*32+4]==1 && nt[4*32+12]==2 && nt[4*32+20]==3);
    step(s,(gb_port_keys_t){.a=true,.b=true,.select=true,.start=true,.up=true,.left=true},4);
    assert(nt[19*32+15]=='1'); // Select 按住不重复增加。
    const char *bits="11111010";
    for(int i=0;i<8;i++) assert(nt[13*32+4+i*3]==(uint8_t)bits[i]);
    int16_t pcm[267]; size_t n=gb_port_audio_frame(s,pcm,267); bool audible=false;
    for(size_t i=0;i<n;i++) audible |= pcm[i]!=0;
    assert(audible);
    step(s,(gb_port_keys_t){.down=true,.right=true},4);
    for(int i=0;i<8;i++) assert(nt[13*32+4+i*3]==(uint8_t)"00000101"[i]);
    step(s,(gb_port_keys_t){.select=true},2);
    assert(nt[19*32+15]=='2');
    uint8_t save[8192];
    assert(gb_port_read_sram(s,0,save,sizeof(save)));
    assert(save[0]==0xa5 && save[1]==2);
    gb_port_destroy(s);
    assert(gb_port_create(&source,&s));
    assert(gb_port_write_sram(s,0,save,sizeof(save)));
    step(s,(gb_port_keys_t){0},120);
    nt=ppu_getnametable(0);
    assert(!memcmp(nt+21*32+4,"LOADED SAVE",11));
    assert(nt[19*32+15]=='2');
    gb_port_destroy(s); free(rom);
    puts("NES2_DIAGNOSTIC_PASS");
}
