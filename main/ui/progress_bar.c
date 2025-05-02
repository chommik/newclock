/*
    Newclock
    ui/progress_bar.c

    Copyright (c) 2025 Rafal Macyszyn

    SPDX-License-Identifier: BSD-3-Clause
*/

#include "progress_bar.h"

#include <math.h>
#include <stdint.h>
#include <sys/param.h>

#include "u8g2.h"

void ui_draw_progress_bar(u8g2_t *u8g2, uint16_t x, uint16_t y, uint16_t r, uint16_t width, uint16_t height,
    uint16_t bar_max, uint16_t bar_value)
{
    if (bar_value > bar_max) {
        bar_value = bar_max;
    }

    uint16_t bar_value_px = floor(((float)bar_value / (double)bar_max) * width);
    uint16_t remaining_bar_rect_width = MIN(MAX(bar_value_px - r, 0), width - 2 * r);

    if (bar_value_px >= r) {
        u8g2_DrawDisc(u8g2, x + r, y + r, r, U8G2_DRAW_UPPER_LEFT);
        u8g2_DrawDisc(u8g2, x + r, y + height - r, r, U8G2_DRAW_LOWER_LEFT);
        u8g2_DrawBox(u8g2, x, y + r, r, height - 2 * r);
    } else {
        u8g2_DrawCircle(u8g2, x + r, y + r, r, U8G2_DRAW_UPPER_LEFT);
        u8g2_DrawCircle(u8g2, x + r, y + height - r, r, U8G2_DRAW_LOWER_LEFT);
    }

    u8g2_DrawVLine(u8g2, x, y + r, height - 2 * r);
    u8g2_DrawVLine(u8g2, x + width, y + r, height - 2 * r);

    u8g2_DrawHLine(u8g2, x + r, y, width - 2 * r);
    u8g2_DrawHLine(u8g2, x + r, y + height, width - 2 * r);

    if (remaining_bar_rect_width > 0) {
        u8g2_DrawBox(u8g2, x + r, y, remaining_bar_rect_width, height);
    }

    if (bar_value_px >= width - r) {
        u8g2_DrawDisc(u8g2, x + width - r, y + r, r, U8G2_DRAW_UPPER_RIGHT);
        u8g2_DrawDisc(u8g2, x + width - r, y + height - r, r, U8G2_DRAW_LOWER_RIGHT);
        u8g2_DrawBox(u8g2, x + width - r, y + r, r, height - 2 * r);
    } else {
        u8g2_DrawCircle(u8g2, x + width - r, y + r, r, U8G2_DRAW_UPPER_RIGHT);
        u8g2_DrawCircle(u8g2, x + width - r, y + height - r, r, U8G2_DRAW_LOWER_RIGHT);
    }
}