#pragma once

#include "u8g2.h"
#include "u8x8.h"
#include "st7920_cmds.h"

void u8x8_Setup_st7920_128x64_esp32(u8x8_t *u8x8);

void u8g2_SetupBuffer_st7920_128x64_esp32(u8g2_t *u8g2, const u8g2_cb_t *u8g2_cb, struct st7920_display *display);
