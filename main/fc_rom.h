#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef struct {
    uint32_t prg_offset, prg_bytes, chr_offset, chr_bytes;
    uint16_t mapper;
    uint32_t prg_ram_bytes, prg_nvram_bytes, chr_ram_bytes;
    bool nes2, battery, trainer, vertical, four_screen;
} fc_rom_info_t;
typedef enum {
    FC_ROM_OK, FC_ROM_HEADER, FC_ROM_CONSOLE, FC_ROM_MAPPER, FC_ROM_SUBMAPPER,
    FC_ROM_REGION, FC_ROM_SIZE, FC_ROM_RAM, FC_ROM_DEVICE, FC_ROM_MISC,
    FC_ROM_RESERVED, FC_ROM_LENGTH, FC_ROM_TRAINER
} fc_rom_error_t;
// 上传预检和加载必须使用同一规则；仅接受现有核心能够处理的布局。
fc_rom_error_t fc_rom_validate(const uint8_t *header, size_t header_bytes,
                             size_t file_bytes, fc_rom_info_t *out);
const char *fc_rom_error_string(fc_rom_error_t error);
bool fc_rom_parse(const uint8_t *header, size_t header_bytes, size_t file_bytes,
                  fc_rom_info_t *out);
size_t fc_rom_allocation(size_t bytes);
