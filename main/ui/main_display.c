/*
    Newclock
    ui/main_display.c

    Copyright (c) 2025 Rafal Macyszyn

    SPDX-License-Identifier: BSD-3-Clause
*/

#include "esp_log.h"
#include "freertos/idf_additions.h"
#include <freertos/FreeRTOS.h>

#include <math.h>
#include <time.h>
#include <u8g2.h>

#include "button_handling.h"
#include "temperature.h"
#include "u8g2_font_maniacbold.h"
#include "ui/ui.h"
#include "utils/utils.h"

#define STRING_BUF_LEN 128
static char string_buf[STRING_BUF_LEN];

static const char *TAG = "ui_main";

static struct ui_screen_t *requested_next_screen = NULL;

enum ui_state_t ui_main_screen_draw(u8g2_t *u8g2)
{
    struct tm tm_now;
    time_t time_now;

    time(&time_now);
    localtime_r(&time_now, &tm_now);

    // u8g2_SetFont(&u8g2, u8g2_font_maniac_tn);
    u8g2_SetFont(u8g2, u8g2_font_maniacbold_tn);
    u8g2_SetFontDirection(u8g2, 0);

    // Here's a trick - calculate the margins using one font, but the "bold" and regular ones are exactly the same widths
    snprintf(string_buf, STRING_BUF_LEN, "%02d:%02d:%02d", tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
    uint8_t margin = (128 - u8g2_GetStrWidth(u8g2, string_buf) - 4) / 2;

    snprintf(string_buf, STRING_BUF_LEN, "%02d:%02d:%02d", tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
    u8g2_DrawStr(u8g2, 2 + margin, 60, string_buf);

    // u8g2_font_streamline_all_t:      dom: 003e, zewn: 00db, pogoda: 02b6
    // u8g2_font_iconquadpix_m_all:     dom: 006a, zewn: 0066, pogoda: ----

    u8g2_SetFont(u8g2, u8g2_font_iconquadpix_m_all);
    u8g2_DrawGlyph(u8g2, 2, 12 /* height */ + 3 /* top margin */, 0x006a);
    u8g2_DrawGlyph(u8g2, 64, 12 /* height */ + 3 /* top margin */, 0x0066);

    u8g2_SetFont(u8g2, u8g2_font_helvR08_te);

    float temperature;

    // inside temperature
    temperature = get_temperature_for_sensor(0x9a0316820c31ff28);
    if (isnan(temperature)) {
        u8g2_DrawUTF8(u8g2, 2 + 14, 10 /* height */ + 3 /* top margin */, "--,-°C");
    } else {
        snprintf(string_buf, STRING_BUF_LEN, "%+0.1f°C", temperature);
        str_nreplace(string_buf, STRING_BUF_LEN, '.', ',');
        u8g2_DrawUTF8(u8g2, 2 + 14, 10 /* height */ + 3 /* top margin */, string_buf);
    }

    temperature = get_temperature_for_sensor(0x220416c182f1ff28);
    if (isnan(temperature)) {
        u8g2_DrawUTF8(u8g2, 64 + 14, 10 /* height */ + 3 /* top margin */, "--,-°C");
    } else {
        snprintf(string_buf, STRING_BUF_LEN, "%+0.1f°C", temperature);
        str_nreplace(string_buf, STRING_BUF_LEN, '.', ',');
        u8g2_DrawUTF8(u8g2, 64 + 14, 10 /* height */ + 3 /* top margin */, string_buf);
    }

    u8g2_SetFont(u8g2, u8g2_font_helvR08_te);

    format_tm_polish(tm_now, string_buf, STRING_BUF_LEN);
    uint8_t date_width = u8g2_GetUTF8Width(u8g2, string_buf);
    u8g2_DrawUTF8(u8g2, (128 - 2 - date_width) / 2, 32, string_buf);

    return UI_STATE_IDLE;
}

TickType_t ui_main_screen_wait_time(void) { return 1000 / portTICK_PERIOD_MS; }

enum ui_state_t ui_main_screen_process_events(void)
{
    struct btn_event_t event;
    enum ui_state_t ret = UI_STATE_REDRAW;

    while (xQueueReceive(g_button_event_queue, &event, 0) == pdPASS) {
        ESP_LOGI(TAG, "event: %s %s", btn_action_t_str(event.btn_action), btn_id_t_str(event.btn_id));

        if (event.btn_action == BTN_ACT_UP && event.btn_id == BTN_5) {
            requested_next_screen = &ui_config_screen;
            ret = UI_STATE_DEINIT_SCREEN;
            xQueueReset(g_button_event_queue);
        }
    }

    return ret;
}

enum ui_state_t ui_main_init_function(u8g2_t *u8g2)
{
    u8g2_SetFontMode(u8g2, 0);
    u8g2_SetDrawColor(u8g2, 1);
    return UI_STATE_REDRAW;
}

struct ui_screen_t *ui_main_deinit_function(void) { return requested_next_screen; }

struct ui_screen_t ui_main_screen = {
    .name = "main_screen",
    .init = ui_main_init_function,
    .deinit = ui_main_deinit_function,
    .draw = ui_main_screen_draw,
    .process_events = ui_main_screen_process_events,
    .wait_time = ui_main_screen_wait_time,
};