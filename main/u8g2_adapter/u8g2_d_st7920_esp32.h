/*
    Newclock
    u8g2_adapter/u8g2_d_st7920_esp32.h

    Copyright (c) 2025 Rafal Macyszyn

    Original design from u8g2 library by olikraus@gmail.com in sys/tga/common/u8x8_d_tga.c.

    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "u8g2.h"
#include "u8x8.h"
#include "st7920_cmds.h"

void u8x8_Setup_st7920_128x64_esp32(u8x8_t *u8x8);

void u8g2_SetupBuffer_st7920_128x64_esp32(u8g2_t *u8g2, const u8g2_cb_t *u8g2_cb, struct st7920_display *display);
