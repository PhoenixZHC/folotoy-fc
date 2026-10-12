#include "fc_rom.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

static fc_rom_info_t info;
static void rejected(uint8_t *h, unsigned at, uint8_t value, fc_rom_error_t error) {
    uint8_t saved=h[at]; h[at]=value;
    assert(fc_rom_validate(h,16,24592,&info)==error);
    h[at]=saved;
}
int main(void) {
    uint8_t h[16]={'N','E','S',0x1a,1,1,0,8};
    assert(fc_rom_validate(h,16,24592,&info)==FC_ROM_OK);
    assert(info.nes2 && !info.prg_ram_bytes && !info.prg_nvram_bytes && !info.battery);
    h[10]=7; assert(fc_rom_parse(h,16,24592,&info));
    assert(info.prg_ram_bytes==8192 && !info.prg_nvram_bytes);
    h[6]=2; h[10]=0x70; assert(fc_rom_parse(h,16,24592,&info));
    assert(info.prg_nvram_bytes==8192 && !info.prg_ram_bytes);
    rejected(h,8,0x10,FC_ROM_SUBMAPPER);
    rejected(h,8,1,FC_ROM_MAPPER);
    rejected(h,7,9,FC_ROM_CONSOLE);
    rejected(h,12,1,FC_ROM_REGION);
    rejected(h,12,3,FC_ROM_REGION);
    h[12]=2; assert(fc_rom_parse(h,16,24592,&info)); h[12]=0;
    rejected(h,12,4,FC_ROM_RESERVED);
    rejected(h,14,1,FC_ROM_MISC);
    rejected(h,15,2,FC_ROM_DEVICE);
    h[15]=1; assert(fc_rom_parse(h,16,24592,&info));
    rejected(h,10,0x77,FC_ROM_RAM);
    rejected(h,10,0x60,FC_ROM_RAM);
    rejected(h,10,0,FC_ROM_RAM);
    rejected(h,11,0x70,FC_ROM_RAM);
    rejected(h,11,7,FC_ROM_RAM);
    assert(fc_rom_validate(h,16,24591,&info)==FC_ROM_LENGTH);
    assert(fc_rom_validate(h,16,24593,&info)==FC_ROM_LENGTH);
    // 指数声明：16 KiB PRG 和 8 KiB CHR，与普通声明完全等价。
    h[4]=14*4; h[5]=13*4; h[9]=0xff;
    assert(fc_rom_parse(h,16,24592,&info));
    assert(info.prg_bytes==16384 && info.chr_bytes==8192);
    rejected(h,4,255,FC_ROM_SIZE);
    rejected(h,5,255,FC_ROM_SIZE);
    rejected(h,4,13*4,FC_ROM_SIZE);
    // Mapper 4 的 1.5 MiB 可由指数乘数表达，不能绕过原容量上限。
    h[4]=19*4+1; h[5]=0; h[9]=15; h[6]=0x42; h[11]=7;
    assert(fc_rom_parse(h,16,16+1572864,&info));
    h[4]=20*4; assert(fc_rom_parse(h,16,16+1048576,&info));
    h[4]=21*4; assert(fc_rom_validate(h,16,16+2097152,&info)==FC_ROM_SIZE);
    h[9]=1; h[4]=0; assert(fc_rom_validate(h,16,16+4194304,&info)==FC_ROM_SIZE);
    h[9]=0; h[4]=2; h[6]=4; h[10]=0;
    assert(fc_rom_validate(h,16,16+512+32768,&info)==FC_ROM_TRAINER);
    for(int e=0;e<=FC_ROM_TRAINER;e++) assert(strlen(fc_rom_error_string(e)));
    puts("FC_ROM_PASS");
    return 0;
}
