#include "fc_rom.h"
#include <string.h>

size_t fc_rom_allocation(size_t bytes) {
    if (bytes > SIZE_MAX - 4095u) return 0;
    return (bytes + 4095u) & ~(size_t)4095u;
}

bool fc_rom_parse(const uint8_t *h, size_t count, size_t size, fc_rom_info_t *out) {
    if (!h || count < 16 || !out || memcmp(h, "NES\x1a", 4) || !h[4]) return false;
    if ((h[7] & 15) || h[9] || h[10] || h[11] || h[12] || h[13] || h[14] || h[15]) return false;
    if (h[8] > 1) return false; // 当前仅支持 8 KiB 卡带 RAM。
    fc_rom_info_t r = {
        .mapper = (h[6] >> 4) | (h[7] & 0xf0),
        .prg_offset = 16 + ((h[6] & 4) ? 512 : 0),
        .prg_bytes = (uint32_t)h[4] * 16384,
        .chr_bytes = (uint32_t)h[5] * 8192,
        .battery = (h[6] & 2) != 0, .trainer = (h[6] & 4) != 0,
        .vertical = (h[6] & 1) != 0, .four_screen = (h[6] & 8) != 0,
    };
    switch (r.mapper) {
    case 0: if (h[4] > 2 || h[5] > 1) return false; break;
    case 1: if (h[4] > 16 || h[5] > 16) return false; break;
    case 2: if (h[4] < 2 || h[4] > 16 || h[5] != 0) return false; break;
    case 3: if (h[4] > 2 || h[5] == 0 || h[5] > 4) return false; break;
    case 4: if (h[4] < 2 || h[4] > 32 || h[5] > 32) return false; break;
    case 7: if (h[4] < 2 || h[4] % 2 || h[4] > 16 || h[5]) return false; break;
    case 23: if (h[4] < 2 || h[4] > 32 || h[5] == 0 || h[5] > 32) return false; break;
    case 66: if (h[4] < 2 || h[4] % 2 || h[4] > 8 || h[5] == 0 || h[5] > 4) return false; break;
    default: return false;
    }
    r.chr_offset = r.prg_offset + r.prg_bytes;
    if (size != (size_t)r.chr_offset + r.chr_bytes) return false;
    *out = r;
    return true;
}
