#pragma once

#include <stdbool.h>
#include <stdint.h>

// 只旋转游戏像素，面板坐标及全部菜单仍为240x320竖屏。
enum { FC_VIDEO_PORTRAIT, FC_VIDEO_LANDSCAPE, FC_VIDEO_FLIPPED, FC_VIDEO_COUNT };
static inline int fc_video_height(uint8_t orientation) { return orientation ? 256 : 225; }
static inline int fc_video_top(uint8_t orientation) { return orientation ? 32 : 40; }

static inline void fc_video_row(uint8_t *const *frame, const uint16_t wire[64],
                                int row, uint8_t orientation, uint16_t out[240]) {
    if (orientation == FC_VIDEO_FLIPPED) {
        // 横屏翻转：逆时针90度，相对普通横屏旋转180度。
        for (int x = 0; x < 240; ++x)
            out[x] = wire[frame[x][8 + 255 - row] & 63];
    } else if (orientation == FC_VIDEO_LANDSCAPE) {
        // 原始256x240顺时针旋转，物理画面240x256，四周不裁切。
        for (int x = 0; x < 240; ++x)
            out[x] = wire[frame[239 - x][8 + row] & 63];
    } else {
        const uint8_t *src = frame[row * 240 / 225] + 8;
        for (int x = 0; x < 240; ++x)
            out[x] = wire[src[x * 256 / 240] & 63];
    }
}
