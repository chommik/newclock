#include "buttons.h"

#include <math.h>
#include <stdint.h>
#include <sys/param.h>

#include "u8g2.h"

uint8_t u8g2_GetFontCapitalAHeight(u8g2_t *u8g2) U8G2_NOINLINE;

void ui_draw_buttons(u8g2_t *u8g2, const ui_buttons_t *buttons)
{
    uint16_t total_space = buttons->right_x - buttons->left_x;

    uint16_t forced_width = 0;
    uint16_t auto_width_button_count = buttons->button_count;

    for (size_t i = 0; i < buttons->button_count; i++) {
        ui_button_definition_t button = buttons->buttons[i];
        if (button.forced_width) {
            forced_width += button.forced_width;
            auto_width_button_count--;
        }
    }

    uint16_t button_auto_width = floor((float)(total_space - forced_width - MAX((buttons->button_count - 1) * buttons->margin_between, 0)) / auto_width_button_count);
    uint16_t current_x = buttons->left_x;

    for (size_t i = 0; i < buttons->button_count; i++) {
        ui_button_definition_t button = buttons->buttons[i];
        uint16_t button_width = button.forced_width ? button.forced_width : button_auto_width;

        if (button.state != UI_BUTTON_STATE_EMPTY) {
            uint16_t text_width = u8g2_GetUTF8Width(u8g2, button.label);
            uint16_t text_margin_left = (button_width - text_width) / 2;
            uint16_t text_margin_top = (buttons->height - u8g2_GetFontCapitalAHeight(u8g2)) / 2;

            u8g2_SetFontMode(u8g2, 0);

            switch (button.state) {
                case UI_BUTTON_STATE_ENABLED:
                    u8g2_SetDrawColor(u8g2, 1);
                    u8g2_DrawRFrame(u8g2, current_x, buttons->top_y, button_width, buttons->height, 4);
                    u8g2_DrawUTF8(u8g2, current_x + text_margin_left, buttons->top_y + buttons->height - text_margin_top,
                                 button.label);
                    break;
                case UI_BUTTON_STATE_ACTIVE:
                    u8g2_SetDrawColor(u8g2, 1);
                    u8g2_DrawRBox(u8g2, current_x, buttons->top_y, button_width, buttons->height, 4);
                    u8g2_SetDrawColor(u8g2, 0);
                    u8g2_DrawUTF8(u8g2, current_x + text_margin_left, buttons->top_y + buttons->height - text_margin_top,
                                 button.label);
                    break;
                default: break;
            }
        }

        current_x += button_width + buttons->margin_between;
    }
}