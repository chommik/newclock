#pragma once

#include <stdint.h>
#include <u8g2.h>

void ui_draw_progress_bar(u8g2_t *u8g2, uint16_t x, uint16_t y, uint16_t r, uint16_t width, uint16_t height, uint16_t bar_max, uint16_t bar_value);