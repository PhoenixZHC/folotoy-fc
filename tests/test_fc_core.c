#include "gb_port.h"
#include "fc_rom.h"
#include "gb_input.h"
#include "nes/nes.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    assert(argc == 2);
    FILE *f=fopen(argv[1],"rb"); assert(f);
    fseek(f,0,SEEK_END); long size=ftell(f); rewind(f);
    uint8_t *rom=malloc(size); assert(rom);
    assert(fread(rom,1,size,f)==(size_t)size); fclose(f);
    fc_rom_info_t info;
    assert(fc_rom_parse(rom,size,size,&info));
    assert(info.mapper==0 && info.prg_bytes==16384 && info.chr_bytes==8192 && info.battery);
    assert(!fc_rom_parse(rom,15,size,&info));
    assert(!fc_rom_parse(rom,size,size-1,&info));
    assert(!fc_rom_parse(rom,size,size+1,&info));
    rom[7]=8; assert(!fc_rom_parse(rom,size,size,&info)); rom[7]=0;
    rom[6]=0xf0; assert(!fc_rom_parse(rom,size,size,&info)); rom[6]=2;
    rom[9]=1; assert(!fc_rom_parse(rom,size,size,&info)); rom[9]=0;
    assert(fc_rom_allocation(24592)==28672);
    assert(fc_rom_allocation(SIZE_MAX)==0);
    {
        // Mapper 4 接受 1.5 MiB PRG，仍拒绝越界容量和截断文件。
        uint8_t header[16]={'N','E','S',0x1a,96,0,0x42};
        assert(fc_rom_parse(header,sizeof(header),16+1572864,&info));
        assert(info.mapper==4 && info.prg_bytes==1572864 && info.chr_bytes==0 && info.battery);
        assert(!fc_rom_parse(header,sizeof(header),16+1572864-1,&info));
        header[4]=97;
        assert(!fc_rom_parse(header,sizeof(header),16+97*16384,&info));
    }
    {
        // 验证上限内高位 PRG 块及倒数两个固定块确实可寻址。
        size_t large_size=16+1572864;
        uint8_t *large=calloc(1,large_size); assert(large);
        memcpy(large,"NES\x1a",4); large[4]=96; large[6]=0x40;
        for (unsigned bank=0;bank<192;bank++) memset(large+16+bank*8192,bank,8192);
        gb_session_t *s=NULL;
        gb_rom_source_t source={.mapped=large,.size=(uint32_t)large_size};
        assert(gb_port_create(&source,&s));
        assert(mem_getbyte(0xc000)==190 && mem_getbyte(0xe000)==191);
        mem_putbyte(0x8000,6); mem_putbyte(0x8001,128);
        assert(mem_getbyte(0x8000)==128);
        mem_putbyte(0x8000,7); mem_putbyte(0x8001,191);
        assert(mem_getbyte(0xa000)==191);
        mem_putbyte(0x8000,0x46); mem_putbyte(0x8001,189);
        assert(mem_getbyte(0x8000)==190 && mem_getbyte(0xc000)==189);
        gb_port_destroy(s); free(large);
    }
    {
        gb_session_t *s=NULL;
        gb_rom_source_t source={.mapped=rom,.size=(uint32_t)size};
        assert(gb_port_create(&source,&s));
        // 实际 NES $4016 串行接口应读到方向+A/B组合以及独立松开。
        gb_input_buffer_t input = {0};
        gb_input_buffer_reset(&input, (pad_sample_t){.connected=true}, true);
        const uint8_t chord[] = {0x07,0x0d,0x0e};
        const uint8_t moving[] = {0x07};
        gb_input_buffer_push(&input, gb_input_keyboard(chord,3,0), 1000);
        gb_input_buffer_push(&input, gb_input_keyboard(moving,1,0), 2000);
        for (int frame=0; frame<2; frame++) {
            gb_keys_t k=gb_input_map(gb_input_buffer_take(&input,16000*(frame+1)));
            gb_port_set_keys(s,(gb_port_keys_t){.right=k.right,.a=k.a,.b=k.b});
            input_write(0x4016,1); input_write(0x4016,0);
            unsigned bits=0;
            for (unsigned bit=0; bit<8; bit++) bits|=(input_read(0x4016)&1u)<<bit;
            assert(bits==(unsigned)(NES_PAD_RIGHT | (frame==0 ? NES_PAD_A|NES_PAD_B : 0)));
        }
        // 未写 PPUCTRL 就载入显存时，每次 PPUDATA 写入仍必须递增地址。
        ppu_write(0x2001,0);
        ppu_write(0x2006,0x20); ppu_write(0x2006,0);
        ppu_write(0x2007,0x12); ppu_write(0x2007,0x34);
        assert(nes_getptr()->ppu->vaddr==0x2002);
        assert(ppu_getnametable(0)[0]==0x12 && ppu_getnametable(0)[1]==0x34);
        ppu_write(0x2000,4);
        assert(nes_getptr()->ppu->vaddr_inc==32);
        nes_reset(true);
        assert(nes_getptr()->ppu->vaddr_inc==1);
        gb_port_destroy(s);
    }
    for (int round=0; round<5; round++) {
        gb_session_t *s=NULL,*second=NULL;
        gb_rom_source_t source={.mapped=rom,.size=(uint32_t)size};
        assert(gb_port_create(&source,&s)); assert(s);
        assert(!gb_port_create(&source,&second)); assert(second==NULL);
        size_t samples=0; bool audible=false;
        for (int frame=0; frame<120; frame++) {
            gb_port_set_keys(s,(gb_port_keys_t){.a=frame>=60});
            assert(gb_port_step_frame(s,round%2==0 || frame%2==0));
            int16_t pcm[267]; size_t n=gb_port_audio_frame(s,pcm,267);
            assert(n==266 || n==267); samples+=n;
            for(size_t i=0;i<n;i++) audible |= pcm[i]!=0;
            if(frame==59 || frame==119) {
                uint8_t button; assert(gb_port_read_sram(s,0,&button,1));
                assert(button==(frame>=60));
            }
        }
        assert(samples==32000); assert(audible);
        uint8_t *const *pixels=gb_port_framebuffer(s);
        bool varied=false;
        for(int i=8;i<256;i++) varied|=(pixels[100][i]&63)!=(pixels[100][8]&63);
        assert(varied); assert(fc_port_palette(s));
        // 异步 LCD 借用帧期间，跳过绘制的模拟帧不得修改像素或调色板。
        uint8_t *snapshot=malloc(NES_SCREEN_PITCH*NES_SCREEN_HEIGHT); assert(snapshot);
        for(int y=0;y<NES_SCREEN_HEIGHT;y++) memcpy(snapshot+y*NES_SCREEN_PITCH,pixels[y],NES_SCREEN_PITCH);
        uint16_t colors[64]; memcpy(colors,fc_port_palette(s),sizeof(colors));
        for(int skip=0;skip<8;skip++) assert(gb_port_step_frame(s,false));
        for(int y=0;y<NES_SCREEN_HEIGHT;y++) assert(memcmp(snapshot+y*NES_SCREEN_PITCH,pixels[y],NES_SCREEN_PITCH)==0);
        assert(memcmp(colors,fc_port_palette(s),sizeof(colors))==0);
        free(snapshot);
        gb_port_mark_saved(s); assert(!gb_port_needs_save(s));
        uint8_t byte=0x42; assert(gb_port_write_sram(s,8000,&byte,1));
        assert(gb_port_needs_save(s));
        assert(!gb_port_write_sram(s,8192,&byte,1));
        assert(!gb_port_read_sram(s,SIZE_MAX,&byte,1));
        gb_port_destroy(s);
    }
    free(rom);
    puts("FC_CORE_PASS: parser, CPU, PPU, APU, input, SRAM, repeated lifecycle");
    return 0;
}
