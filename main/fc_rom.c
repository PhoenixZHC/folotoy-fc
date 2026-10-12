#include "fc_rom.h"
#include <string.h>

size_t fc_rom_allocation(size_t bytes) {
    if (bytes > SIZE_MAX - 4095u) return 0;
    return (bytes + 4095u) & ~(size_t)4095u;
}

// 移位前检查指数和乘数，避免损坏文件头导致溢出。
static bool rom_size(uint8_t low, unsigned high, uint32_t unit, uint32_t *out) {
    if (high != 15) { *out = ((high << 8) | low) * unit; return true; }
    unsigned exponent = low >> 2, multiplier = (low & 3) * 2 + 1;
    if (exponent >= 32 || (UINT32_MAX >> exponent) < multiplier) return false;
    *out = (uint32_t)multiplier << exponent;
    return true;
}
static uint32_t ram_size(unsigned shift) { return shift ? 64u << shift : 0; }

fc_rom_error_t fc_rom_validate(const uint8_t *h, size_t count, size_t size, fc_rom_info_t *out) {
    if (!h || count < 16 || !out || memcmp(h, "NES\x1a", 4)) return FC_ROM_HEADER;
    fc_rom_info_t r = {
        .mapper = (h[6] >> 4) | (h[7] & 0xf0), .nes2 = (h[7] & 12) == 8,
        .prg_offset = 16 + ((h[6] & 4) ? 512 : 0),
        .battery = (h[6] & 2) != 0, .trainer = (h[6] & 4) != 0,
        .vertical = (h[6] & 1) != 0, .four_screen = (h[6] & 8) != 0,
    };
    if (h[7] & 3) return FC_ROM_CONSOLE;
    if (r.nes2) {
        r.mapper |= (h[8] & 15) << 8;
        if (h[8] >> 4) return FC_ROM_SUBMAPPER;
        if ((h[12] & 0xfc) || h[13] || (h[14] & 0xfc) || (h[15] & 0xc0)) return FC_ROM_RESERVED;
        if ((h[12] & 3) != 0 && (h[12] & 3) != 2) return FC_ROM_REGION;
        if (h[14] & 3) return FC_ROM_MISC;
        if (h[15] > 1) return FC_ROM_DEVICE;
        if (!rom_size(h[4], h[9] & 15, 16384, &r.prg_bytes) ||
            !rom_size(h[5], h[9] >> 4, 8192, &r.chr_bytes)) return FC_ROM_SIZE;
        r.prg_ram_bytes = ram_size(h[10] & 15);
        r.prg_nvram_bytes = ram_size(h[10] >> 4);
        r.chr_ram_bytes = ram_size(h[11] & 15);
        // 单个 8 KiB PRG RAM 窗口；不支持混合 RAM 或 CHR 存档。
        if ((r.prg_ram_bytes && r.prg_ram_bytes != 8192) ||
            (r.prg_nvram_bytes && r.prg_nvram_bytes != 8192) ||
            (r.prg_ram_bytes && r.prg_nvram_bytes) || (h[11] >> 4) ||
            r.battery != (r.prg_nvram_bytes != 0) ||
            r.chr_ram_bytes != (r.chr_bytes ? 0u : 8192u)) return FC_ROM_RAM;
    } else {
        if ((h[7] & 12) || h[10] || h[11] || h[12] || h[13] || h[14] || h[15]) return FC_ROM_HEADER;
        if (h[9]) return FC_ROM_REGION;
        if (h[8] > 1) return FC_ROM_RAM;
        r.prg_bytes = (uint32_t)h[4] * 16384;
        r.chr_bytes = (uint32_t)h[5] * 8192;
        r.prg_ram_bytes = r.battery ? 0 : 8192;
        r.prg_nvram_bytes = r.battery ? 8192 : 0;
        r.chr_ram_bytes = r.chr_bytes ? 0 : 8192;
    }
    if (!r.prg_bytes || r.prg_bytes % 16384 || r.chr_bytes % 8192) return FC_ROM_SIZE;
    uint32_t p = r.prg_bytes / 16384, c = r.chr_bytes / 8192;
    bool shape;
    switch (r.mapper) {
    case 0: shape = p <= 2 && c <= 1; break;
    case 1: shape = p <= 16 && c <= 16; break;
    case 2: shape = p >= 2 && p <= 16 && c == 0; break;
    case 3: shape = p <= 2 && c >= 1 && c <= 4; break;
    case 4: shape = p >= 2 && p <= 96 && c <= 32; break;
    case 7: shape = p >= 2 && p <= 16 && p % 2 == 0 && c == 0; break;
    case 23: shape = p >= 2 && p <= 32 && c >= 1 && c <= 32; break;
    case 66: shape = p >= 2 && p <= 8 && p % 2 == 0 && c >= 1 && c <= 4; break;
    default: return FC_ROM_MAPPER;
    }
    if (!shape) return FC_ROM_SIZE;
    if (r.trainer && !r.prg_ram_bytes && !r.prg_nvram_bytes) return FC_ROM_TRAINER;
    r.chr_offset = r.prg_offset + r.prg_bytes;
    if (size != (size_t)r.chr_offset + r.chr_bytes) return FC_ROM_LENGTH;
    *out = r;
    return FC_ROM_OK;
}
bool fc_rom_parse(const uint8_t *h, size_t count, size_t size, fc_rom_info_t *out) {
    return fc_rom_validate(h, count, size, out) == FC_ROM_OK;
}
const char *fc_rom_error_string(fc_rom_error_t error) {
    switch (error) {
    case FC_ROM_OK: return "格式有效";
    case FC_ROM_HEADER: return "文件头无效";
    case FC_ROM_CONSOLE: return "不支持此主机类型";
    case FC_ROM_MAPPER: return "不支持此卡带类型";
    case FC_ROM_SUBMAPPER: return "不支持此硬件子型号";
    case FC_ROM_REGION: return "不支持此区域制式";
    case FC_ROM_SIZE: return "不支持此容量布局";
    case FC_ROM_RAM: return "不支持此内存配置";
    case FC_ROM_DEVICE: return "不支持此外设配置";
    case FC_ROM_MISC: return "不支持附加ROM";
    case FC_ROM_RESERVED: return "文件头保留位非零";
    case FC_ROM_LENGTH: return "文件长度与声明不符";
    case FC_ROM_TRAINER: return "训练数据缺少RAM";
    default: return "未知格式错误";
    }
}
