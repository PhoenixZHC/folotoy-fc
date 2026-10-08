#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef struct {
    uint32_t prg_offset, prg_bytes, chr_offset, chr_bytes;
    uint8_t mapper;
    bool battery, trainer, vertical, four_screen;
} fc_rom_info_t;
// 首版明确接受经典 iNES/NTSC；不把 NES 2.0、PAL、VS/FDS 当作普通卡带。
bool fc_rom_parse(const uint8_t *header, size_t header_bytes, size_t file_bytes,
                  fc_rom_info_t *out);
size_t fc_rom_allocation(size_t bytes);
