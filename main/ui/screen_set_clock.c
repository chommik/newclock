#include <driver/ledc.h>
#include <esp_log.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <u8g2.h>

#include "button_handling.h"
#include "portmacro.h"
#include "ui.h"
#include "ui/buttons.h"
#include "utils/utils.h"

static const char *TAG = "ui_brightness";

static ui_buttons_t buttons = {
    .top_y = 47,
    .height = 16,

    .left_x = 0,
    .right_x = 127,

    .margin_between = 2,

    .button_count = 5,
    .buttons =
        {
            {
                .state = UI_BUTTON_STATE_ENABLED,
                .label = "H+",
                .forced_width = 20,
            },
            {
                .state = UI_BUTTON_STATE_ENABLED,
                .label = "H-",
                .forced_width = 20,
            },
            {
                .state = UI_BUTTON_STATE_ENABLED,
                .label = "M+",
                .forced_width = 20,
            },
            {
                .state = UI_BUTTON_STATE_ENABLED,
                .label = "M-",
                .forced_width = 20,
            },
            {
                .state = UI_BUTTON_STATE_ENABLED,
                .label = "OK",
            },
        },
};

#define STRING_BUF_LEN 32
static char string_buf[STRING_BUF_LEN];

enum ui_state_t ui_set_clock_init(u8g2_t *u8g2)
{
    u8g2_SetFontMode(u8g2, 0);
    u8g2_SetDrawColor(u8g2, 1);

    return UI_STATE_REDRAW;
}

struct ui_screen_t *ui_set_clock_deinit(void) { return &ui_config_screen; }

enum ui_state_t ui_set_clock_draw(u8g2_t *u8g2)
{
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    u8g2_SetFont(u8g2, u8g2_font_helvB08_te);
    u8g2_DrawUTF8(u8g2, 1, 10, "Czas");

    u8g2_DrawHLine(u8g2, 0, 11, 128);

    u8g2_SetFont(u8g2, u8g2_font_helvB14_tn);

    uint16_t current_x = 30;
    snprintf(string_buf, STRING_BUF_LEN, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    current_x += u8g2_DrawStr(u8g2, current_x, 30, string_buf);

    u8g2_SetFont(u8g2, u8g2_font_helvR10_tn);

    snprintf(string_buf, STRING_BUF_LEN, ":%02d", timeinfo.tm_sec);
    u8g2_DrawStr(u8g2, current_x, 30, string_buf);

    u8g2_SetFont(u8g2, u8g2_font_helvR08_te);
    format_tm_polish(timeinfo, string_buf, STRING_BUF_LEN);
    u8g2_DrawUTF8Line(u8g2, 0, 42, 128, string_buf, 0, 0);

    u8g2_SetFont(u8g2, u8g2_font_NokiaSmallPlain_te);
    ui_draw_buttons(u8g2, &buttons);

    return UI_STATE_IDLE;
}

enum ui_state_t ui_set_clock_process_events(void)
{
    struct btn_event_t event;
    enum ui_state_t ret = UI_STATE_REDRAW;
    uint8_t set_clock_changed = false;

    while (xQueueReceive(g_button_event_queue, &event, 0) == pdPASS) {
        ESP_LOGI(TAG, "event: %s %s", btn_action_t_str(event.btn_action), btn_id_t_str(event.btn_id));

        size_t btn_array_idx = 0;
        switch (event.btn_id) {
            case BTN_1: btn_array_idx = 0; break;
            case BTN_2: btn_array_idx = 1; break;
            case BTN_3: btn_array_idx = 2; break;
            case BTN_4: btn_array_idx = 3; break;
            case BTN_5: btn_array_idx = 4; break;
            default: break;
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

        if (event.btn_action == BTN_ACT_UP) {
            ret = UI_STATE_REDRAW;

            struct timeval time_now;
            gettimeofday(&time_now, NULL);

            switch (event.btn_id) {
                case BTN_1: // H+
                    time_now.tv_sec += 60 * 60;
                    break;
                case BTN_2: // H-
                    time_now.tv_sec -= 60 * 60;
                    break;
                case BTN_3: // M+
                    time_now.tv_sec += 60;
                    break;
                case BTN_4: // M-
                    time_now.tv_sec -= 60;
                    break;
                case BTN_5: // OK
                default:    // not really, but BTN_MAX technically is also a case
                    return UI_STATE_DEINIT_SCREEN;
            }

            settimeofday(&time_now, NULL);
        }
    }

    return ret;
}

TickType_t ui_set_clock_wait_time(void) { return 1000 / portTICK_PERIOD_MS; }

struct ui_screen_t ui_set_clock_screen = {
    .name = "set_clock",
    .init = ui_set_clock_init,
    .deinit = ui_set_clock_deinit,
    .draw = ui_set_clock_draw,
    .process_events = ui_set_clock_process_events,
    .wait_time = ui_set_clock_wait_time,
};