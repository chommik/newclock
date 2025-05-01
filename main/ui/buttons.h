#pragma once

#include <stdint.h>

#include "button_handling.h"
#include "u8g2.h"

typedef enum
{
    UI_BUTTON_STATE_ENABLED,
    UI_BUTTON_STATE_ACTIVE,
    UI_BUTTON_STATE_EMPTY,
} button_state_t;

typedef struct
{
    const char *label;
    button_state_t state;
    uint8_t forced_width;
} ui_button_definition_t;

typedef struct
{
    uint16_t top_y;
    uint16_t height;

    uint16_t left_x;
    uint16_t right_x;

    uint16_t margin_between;

    size_t button_count;
    ui_button_definition_t buttons[8];
} ui_buttons_t;

void ui_draw_buttons(u8g2_t *u8g2, const ui_buttons_t *buttons);