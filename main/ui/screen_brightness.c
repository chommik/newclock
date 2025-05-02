/*
    Newclock
    ui/screen_brightness.c

    Copyright (c) 2025 Rafal Macyszyn

    SPDX-License-Identifier: BSD-3-Clause
*/

#include <driver/ledc.h>
#include <esp_log.h>

#include <stdint.h>
#include <stdio.h>
#include <u8g2.h>

#include "button_handling.h"
#include "tasks/task_brightness.h"
#include "ui/ui.h"
#include "ui/buttons.h"
#include "ui/progress_bar.h"

uint8_t u8g2_GetFontCapitalAHeight(u8g2_t *u8g2) U8G2_NOINLINE;

static const char *TAG = "ui_brightness";

#define STRING_BUF_LEN 32
static char string_buf[STRING_BUF_LEN];

ui_buttons_t buttons = {
    .top_y = 47,
    .height = 16,

    .left_x = 0,
    .right_x = 127,

    .margin_between = 2,

    .button_count = 6,
    .buttons =
        {
            {
                .state = UI_BUTTON_STATE_ENABLED,
                .label = "-",
                .forced_width = 20,
            },
            {
                .state = UI_BUTTON_STATE_ENABLED,
                .label = "+",
                .forced_width = 20,
            },
            {
                .state = UI_BUTTON_STATE_EMPTY,
            },
            {
                .state = UI_BUTTON_STATE_ENABLED,
                .label = "Auto",
                .forced_width = 24,
            },
            {
                .state = UI_BUTTON_STATE_EMPTY,
            },
            {
                .state = UI_BUTTON_STATE_ENABLED,
                .label = "OK",
                .forced_width = 20,
            },
        },
};

enum ui_state_t ui_brightness_init(u8g2_t *u8g2)
{
    u8g2_SetFontMode(u8g2, 0);
    u8g2_SetDrawColor(u8g2, 1);

    return UI_STATE_REDRAW;
}

struct ui_screen_t *ui_brightness_deinit(void) { return &ui_config_screen; }

enum ui_state_t ui_brightness_draw(u8g2_t *u8g2)
{
    brightness_setting_t current_brightness;
    brightness_get_current_setting(&current_brightness);

    // ----- header

    u8g2_SetFont(u8g2, u8g2_font_helvB08_te);

    u8g2_DrawUTF8(u8g2, 1, 10, "Jasność");
    u8g2_DrawHLine(u8g2, 0, 11, 128);

    // ----- arrow

    u8g2_SetFont(u8g2, u8g2_font_siji_t_6x10);

    uint16_t arrow_y_coord = current_brightness.auto_brightness ? 36 : 23;
    u8g2_DrawGlyph(u8g2, 117, arrow_y_coord, 0xe063);

    // ----- bars and labels

    u8g2_SetFont(u8g2, u8g2_font_NokiaSmallPlain_te);

    u8g2_DrawStr(u8g2, 1, 23, "Manual");
    ui_draw_progress_bar(u8g2, 30, 14, 2, 64, 10, BRIGHTNESS_MAX_VALUE, current_brightness.static_brightness);

    u8g2_DrawStr(u8g2, 1, 36, "Auto");
    ui_draw_progress_bar(u8g2, 30, 27, 2, 64, 10, BRIGHTNESS_MAX_VALUE, current_brightness.light_sensor_brightness);

    snprintf(string_buf, STRING_BUF_LEN, "%d/%d", current_brightness.static_brightness, BRIGHTNESS_MAX_VALUE);
    u8g2_DrawStr(u8g2, 96, 23, string_buf);

    snprintf(string_buf, STRING_BUF_LEN, "%d/%d", current_brightness.light_sensor_brightness, BRIGHTNESS_MAX_VALUE);
    u8g2_DrawStr(u8g2, 96, 36, string_buf);

    ui_draw_buttons(u8g2, &buttons);

    return UI_STATE_IDLE;
}

enum ui_state_t ui_brightness_process_events(void)
{
    struct btn_event_t event;
    enum ui_state_t ret = UI_STATE_REDRAW;

    while (xQueueReceive(g_button_event_queue, &event, 0) == pdPASS) {
        ESP_LOGI(TAG, "event: %s %s", btn_action_t_str(event.btn_action), btn_id_t_str(event.btn_id));

        size_t btn_array_idx = 0;
        switch (event.btn_id) {
            case BTN_1: btn_array_idx = 0; break;
            case BTN_2: btn_array_idx = 1; break;
            case BTN_3: btn_array_idx = 3; break;
            case BTN_5: btn_array_idx = 5; break;
            default: btn_array_idx = 2; break;
        }

        if (event.btn_action == BTN_ACT_DOWN) {
            if (buttons.buttons[btn_array_idx].state == UI_BUTTON_STATE_ENABLED) {
                buttons.buttons[btn_array_idx].state = UI_BUTTON_STATE_ACTIVE;
            }
        } else if (event.btn_action == BTN_ACT_UP) {
            if (buttons.buttons[btn_array_idx].state == UI_BUTTON_STATE_ACTIVE) {
                buttons.buttons[btn_array_idx].state = UI_BUTTON_STATE_ENABLED;
            }
        }

        if (event.btn_action == BTN_ACT_UP && event.btn_id == BTN_1) {
            brightness_msg_t msg = {
                .msg_type = MSG_BRIGHTNESS_CHANGE,
                .msg_arg = -1,
            };
            xQueueSend(g_brighness_msg_queue, &msg, 0);
        }

        if (event.btn_action == BTN_ACT_UP && event.btn_id == BTN_2) {
            brightness_msg_t msg = {
                .msg_type = MSG_BRIGHTNESS_CHANGE,
                .msg_arg = 1,
            };
            xQueueSend(g_brighness_msg_queue, &msg, 0);
        }

        if (event.btn_action == BTN_ACT_UP && event.btn_id == BTN_3) {
            brightness_msg_t msg = {
                .msg_type = MSG_BRIGHTNESS_AUTO_TOGGLE,
            };
            xQueueSend(g_brighness_msg_queue, &msg, 0);
        }

        if (event.btn_action == BTN_ACT_UP && event.btn_id == BTN_5) {
            ret = UI_STATE_DEINIT_SCREEN;
            xQueueReset(g_button_event_queue);
        }
    }

    return ret;
}

TickType_t ui_brightness_wait_time(void) { return BRIGHTNESS_LIGHT_SENSOR_REFRESH_DELAY; }

struct ui_screen_t ui_brightness_screen = {
    .name = "brightness",
    .init = ui_brightness_init,
    .deinit = ui_brightness_deinit,
    .draw = ui_brightness_draw,
    .process_events = ui_brightness_process_events,
    .wait_time = ui_brightness_wait_time,
};