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

static const char *TAG = "ui_brightness";

typedef enum
{
    SCREEN_STATE_MAIN,
    SCREEN_STATE_SET_DAY,
    SCREEN_STATE_SET_MONTH,
    SCREEN_STATE_SET_YEAR,
} screen_state_t;

static ui_buttons_t main_buttons = {
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
                .label = "D",
                .forced_width = 20,
            },
            {
                .state = UI_BUTTON_STATE_ENABLED,
                .label = "M",
                .forced_width = 20,
            },
            {
                .state = UI_BUTTON_STATE_ENABLED,
                .label = "R",
                .forced_width = 20,
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

static ui_buttons_t adjust_buttons = {
    .top_y = 47,
    .height = 16,

    .left_x = 0,
    .right_x = 127,

    .margin_between = 2,

    .button_count = 4,
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
                .label = "OK",
                .forced_width = 20,
            },
        },
};

static screen_state_t current_state;
static ui_buttons_t *current_buttons = &main_buttons;

#define STRING_BUF_LEN 32
static char string_buf[STRING_BUF_LEN];

enum ui_state_t ui_set_date_init(u8g2_t *u8g2)
{
    u8g2_SetFontMode(u8g2, 0);
    u8g2_SetDrawColor(u8g2, 1);

    return UI_STATE_REDRAW;
}

struct ui_screen_t *ui_set_date_deinit(void) { return &ui_config_screen; }

enum ui_state_t ui_set_date_draw(u8g2_t *u8g2)
{
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    u8g2_SetFontMode(u8g2, 0);
    u8g2_SetDrawColor(u8g2, 1);

    u8g2_SetFont(u8g2, u8g2_font_helvB08_te);
    u8g2_DrawUTF8(u8g2, 1, 10, "Data");

    u8g2_DrawHLine(u8g2, 0, 11, 128);

    uint16_t block_width = 0;
    uint16_t current_x = 16;

    // ----- day

    u8g2_SetFont(u8g2, current_state == SCREEN_STATE_SET_DAY ? u8g2_font_helvB18_tn : u8g2_font_helvR14_tn);
    snprintf(string_buf, STRING_BUF_LEN, "%02d", timeinfo.tm_mday);
    current_x += u8g2_DrawStr(u8g2, current_x, 36, string_buf);

    u8g2_SetFont(u8g2, u8g2_font_helvB10_tn);
    current_x += u8g2_DrawStr(u8g2, current_x, 36, ".");

    // ----- month

    u8g2_SetFont(u8g2, current_state == SCREEN_STATE_SET_MONTH ? u8g2_font_helvB18_tn : u8g2_font_helvR14_tn);
    snprintf(string_buf, STRING_BUF_LEN, "%02d", timeinfo.tm_mon + 1);
    current_x += u8g2_DrawStr(u8g2, current_x, 36, string_buf);

    u8g2_SetFont(u8g2, u8g2_font_helvB10_tn);
    current_x += u8g2_DrawStr(u8g2, current_x, 36, ".");

    // ----- year

    u8g2_SetFont(u8g2, current_state == SCREEN_STATE_SET_YEAR ? u8g2_font_helvB18_tn : u8g2_font_helvR14_tn);
    snprintf(string_buf, STRING_BUF_LEN, "%04d", timeinfo.tm_year + 1900);
    current_x += u8g2_DrawStr(u8g2, current_x, 36, string_buf);

    u8g2_SetFont(u8g2, u8g2_font_NokiaSmallPlain_te);
    ui_draw_buttons(u8g2, current_buttons);

    return UI_STATE_IDLE;
}

enum ui_state_t ui_set_date_process_events(void)
{
    struct btn_event_t event;
    enum ui_state_t ret = UI_STATE_REDRAW;

    while (xQueueReceive(g_button_event_queue, &event, 0) == pdPASS) {
        ESP_LOGI(TAG, "event: %s %s", btn_action_t_str(event.btn_action), btn_id_t_str(event.btn_id));

        // ---- button color handling

        size_t btn_array_idx = 0;
        if (current_state == SCREEN_STATE_MAIN) {
            switch (event.btn_id) {
                case BTN_1: btn_array_idx = 0; break;
                case BTN_2: btn_array_idx = 1; break;
                case BTN_3: btn_array_idx = 2; break;
                case BTN_4: btn_array_idx = 3; break;
                case BTN_5: btn_array_idx = 4; break;
                default: break;
            }
        } else if (current_state == SCREEN_STATE_SET_DAY || current_state == SCREEN_STATE_SET_MONTH ||
                   current_state == SCREEN_STATE_SET_YEAR) {
            switch (event.btn_id) {
                case BTN_1: btn_array_idx = 0; break;
                case BTN_2: btn_array_idx = 1; break;
                case BTN_3: btn_array_idx = 2; break;
                case BTN_4: btn_array_idx = 2; break;
                case BTN_5: btn_array_idx = 3; break;
                default: break;
            }
        }

        if (event.btn_action == BTN_ACT_DOWN) {
            if (current_buttons->buttons[btn_array_idx].state == UI_BUTTON_STATE_ENABLED) {
                current_buttons->buttons[btn_array_idx].state = UI_BUTTON_STATE_ACTIVE;
            }
        } else if (event.btn_action == BTN_ACT_UP) {
            if (current_buttons->buttons[btn_array_idx].state == UI_BUTTON_STATE_ACTIVE) {
                current_buttons->buttons[btn_array_idx].state = UI_BUTTON_STATE_ENABLED;
            }
        }

        // ----- button action handling, part 1: UI state change

        if (event.btn_action == BTN_ACT_UP && event.btn_id == BTN_5) {
            switch (current_state) {
                case SCREEN_STATE_MAIN: xQueueReset(g_button_event_queue); return UI_STATE_DEINIT_SCREEN;
                case SCREEN_STATE_SET_DAY:
                case SCREEN_STATE_SET_MONTH:
                case SCREEN_STATE_SET_YEAR: current_state = SCREEN_STATE_MAIN; current_buttons = &main_buttons;
            }
            goto no_more_states;
        }

        // ----- button action handling, part 2: main subscreen (date part chooser)

        if (event.btn_action == BTN_ACT_UP && current_state == SCREEN_STATE_MAIN) {
            switch (event.btn_id) {
                case BTN_1:
                    current_state = SCREEN_STATE_SET_DAY;
                    current_buttons = &adjust_buttons;
                    break;
                case BTN_2:
                    current_state = SCREEN_STATE_SET_MONTH;
                    current_buttons = &adjust_buttons;
                    break;
                case BTN_3:
                    current_state = SCREEN_STATE_SET_YEAR;
                    current_buttons = &adjust_buttons;
                    break;
                case BTN_4:
                case BTN_5:
                default: break;
            }

            goto no_more_states;
        }

        // ----- button action handling, part 3: adjust subscreen (update day/month/year)

        if (event.btn_action == BTN_ACT_UP &&
            (current_state == SCREEN_STATE_SET_DAY || current_state == SCREEN_STATE_SET_MONTH ||
                current_state == SCREEN_STATE_SET_YEAR)) {

            time_t time_now;
            struct tm tm_now;
            time(&time_now);
            localtime_r(&time_now, &tm_now);

            int8_t offset_dir = 0;
            switch (event.btn_id) {
                case BTN_1:
                    offset_dir = -1;
                    break;
                case BTN_2:
                    offset_dir = 1;
                    break;
                default: break;
            }

            switch (current_state) {
                case SCREEN_STATE_MAIN: // impossible
                case SCREEN_STATE_SET_DAY:
                    tm_now.tm_mday += offset_dir;
                    break;
                case SCREEN_STATE_SET_MONTH:
                    tm_now.tm_mon += offset_dir;
                    break;
                case SCREEN_STATE_SET_YEAR:
                    tm_now.tm_year += offset_dir;
                    break;
            }

            time_t new_time = mktime(&tm_now);
            struct timeval new_timeval = {
                .tv_sec = new_time,
            };
            settimeofday(&new_timeval, NULL);

            goto no_more_states;
        }
        no_more_states:
    }

    return ret;
}

TickType_t ui_set_date_wait_time(void) { return 1000 / portTICK_PERIOD_MS; }

struct ui_screen_t ui_set_date_screen = {
    .name = "set_date",
    .init = ui_set_date_init,
    .deinit = ui_set_date_deinit,
    .draw = ui_set_date_draw,
    .process_events = ui_set_date_process_events,
    .wait_time = ui_set_date_wait_time,
};