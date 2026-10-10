#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// 仅供保留的旧显示助手使用；FC 帧缓冲使用 272 字节行距，像素取低 6 位。
#define GB_PORT_COLOR_BLANK 64

#ifdef __cplusplus
extern "C" {
#endif

typedef struct gb_session gb_session_t;
typedef bool (*gb_rom_read_fn)(void *context, uint32_t offset, void *dst, size_t bytes);
typedef struct {
    gb_rom_read_fn read;
    void *context;
    uint32_t size;
    const uint8_t *mapped;
} gb_rom_source_t;

typedef struct {
    bool right, left, up, down;
    bool a, b, select, start;
} gb_port_keys_t;

bool gb_port_create(const gb_rom_source_t *rom, gb_session_t **out);
void gb_port_set_keys(gb_session_t *session, gb_port_keys_t keys);
bool gb_port_step_frame(gb_session_t *session, bool render);
// 240 行，每行 272 字节（含左右各 8 字节 overdraw）；行之间不保证连续。
uint8_t *const *gb_port_framebuffer(const gb_session_t *session);
const uint16_t *fc_port_palette(const gb_session_t *session);
// 复制刚完成一帧的 266/267 个 16 kHz 单声道样本，不额外推进 APU。
size_t gb_port_audio_frame(gb_session_t *session, int16_t *out, size_t capacity);
size_t gb_port_sram_size(const gb_session_t *session);
bool gb_port_has_battery(const gb_session_t *session);
bool gb_port_needs_save(const gb_session_t *session);
void gb_port_mark_saved(gb_session_t *session);
bool gb_port_read_sram(const gb_session_t *session, size_t offset, void *dst, size_t bytes);
bool gb_port_write_sram(gb_session_t *session, size_t offset, const void *src, size_t bytes);
void gb_port_destroy(gb_session_t *session);

#ifdef __cplusplus
}
#endif
