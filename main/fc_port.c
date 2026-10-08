// 适配 Nofrendo 的单实例核心。ROM 只读映射到 Flash，只有 PRG/CHR RAM 放入堆。
#include "gb_port.h"
#include "fc_rom.h"
#include "nofrendo.h"
#include "nes/nes.h"

struct gb_session { nes_t *nes; uint8_t *frame; uint16_t *palette; uint32_t saved_crc; bool saved; };
static gb_session_t *active;
static rom_t cartridge;

void rom_free(void) {
    free(cartridge.prg_ram);
    free(cartridge.chr_ram);
    memset(&cartridge, 0, sizeof(cartridge));
}

bool gb_port_create(const gb_rom_source_t *source, gb_session_t **out) {
    if (out) *out = NULL;
    fc_rom_info_t info;
    if (!out || active || !source || !source->mapped ||
        !fc_rom_parse(source->mapped, source->size, source->size, &info)) return false;
    gb_session_t *s = calloc(1, sizeof(*s));
    if (!s) return false;
    s->frame = calloc(NES_SCREEN_PITCH, NES_SCREEN_HEIGHT);
    s->palette = nofrendo_buildpalette(0, 16);
    if (!s->frame || !s->palette) goto fail;
    s->nes = nes_init(SYS_NES_NTSC, 16000, false, NULL);
    if (!s->nes) goto fail;
    cartridge = (rom_t){
        .type = ROM_TYPE_INES, .mapper_number = info.mapper, .system = SYS_NES_NTSC,
        .mirroring = info.four_screen ? PPU_MIRROR_FOUR : info.vertical ? PPU_MIRROR_VERT : PPU_MIRROR_HORI,
        .battery = info.battery, .vertical = info.vertical, .fourscreen = info.four_screen,
        .prg_rom = (uint8_t *)source->mapped + info.prg_offset,
        .chr_rom = info.chr_bytes ? (uint8_t *)source->mapped + info.chr_offset : NULL,
        .prg_rom_banks = info.prg_bytes / 8192, .chr_rom_banks = info.chr_bytes / 8192,
        .prg_ram_banks = 1, .chr_ram_banks = info.chr_bytes ? 0 : 1,
        .prg_ram = calloc(1, 8192), .chr_ram = info.chr_bytes ? NULL : calloc(1, 8192),
    };
    if (!cartridge.prg_ram || (!info.chr_bytes && !cartridge.chr_ram)) goto fail;
    if (nes_insertcart(&cartridge) < 0) { s->nes = NULL; goto fail; }
    if (info.trainer) memcpy(cartridge.prg_ram + 0x1000, source->mapped + 16, 512);
    nes_setvidbuf(s->frame);
    input_connect(0, NES_JOYPAD);
    input_connect(1, NES_NOTHING);
    active = s;
    *out = s;
    return true;
fail:
    if (s->nes) nes_shutdown();
    free(s->frame); free(s->palette); free(s);
    return false;
}

void gb_port_destroy(gb_session_t *s) {
    if (!s || s != active) return;
    nes_shutdown(); free(s->frame); free(s->palette); free(s); active = NULL;
}
void gb_port_set_keys(gb_session_t *s, gb_port_keys_t k) {
    if (s != active || !s) return;
    input_update(0, (k.a ? NES_PAD_A : 0) | (k.b ? NES_PAD_B : 0) |
        (k.select ? NES_PAD_SELECT : 0) | (k.start ? NES_PAD_START : 0) |
        (k.up && !k.down ? NES_PAD_UP : 0) | (k.down && !k.up ? NES_PAD_DOWN : 0) |
        (k.left && !k.right ? NES_PAD_LEFT : 0) | (k.right && !k.left ? NES_PAD_RIGHT : 0));
}
bool gb_port_step_frame(gb_session_t *s, bool render) {
    if (!s || s != active) return false;
    nes_emulate(render); return true;
}
const uint8_t *gb_port_framebuffer(const gb_session_t *s) { return s ? s->frame : NULL; }
const uint16_t *fc_port_palette(const gb_session_t *s) { return s ? s->palette : NULL; }
size_t gb_port_audio_frame(gb_session_t *s, int16_t *out, size_t capacity) {
    if (!s || !out || capacity < (size_t)s->nes->apu->samples_per_frame) return 0;
    size_t count = s->nes->apu->samples_per_frame;
    memcpy(out, s->nes->apu->buffer, count * sizeof(*out)); return count;
}
size_t gb_port_sram_size(const gb_session_t *s) { return s && cartridge.battery ? 8192 : 0; }
bool gb_port_has_battery(const gb_session_t *s) { return gb_port_sram_size(s) != 0; }
static uint32_t ram_crc(void) {
    uint32_t crc = 0xffffffffu;
    for (size_t i = 0; i < 8192; i++) {
        crc ^= cartridge.prg_ram[i];
        for (int bit = 0; bit < 8; bit++) crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1u)));
    }
    return crc ^ 0xffffffffu;
}
bool gb_port_needs_save(const gb_session_t *s) { return gb_port_has_battery(s) && (!s->saved || ram_crc() != s->saved_crc); }
void gb_port_mark_saved(gb_session_t *s) { if (gb_port_has_battery(s)) { s->saved_crc = ram_crc(); s->saved = true; } }
bool gb_port_read_sram(const gb_session_t *s, size_t at, void *dst, size_t bytes) {
    if (!gb_port_has_battery(s) || !dst || at > 8192 || bytes > 8192-at) return false;
    memcpy(dst, cartridge.prg_ram+at, bytes); return true;
}
bool gb_port_write_sram(gb_session_t *s, size_t at, const void *src, size_t bytes) {
    if (!gb_port_has_battery(s) || !src || at > 8192 || bytes > 8192-at) return false;
    memcpy(cartridge.prg_ram+at, src, bytes); return true;
}
