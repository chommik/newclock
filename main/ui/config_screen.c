#include <esp_log.h>

#include <stdint.h>
#include <u8g2.h>

#include "button_handling.h"
#include "portmacro.h"
#include "ui.h"
#include "ui/buttons.h"
#include "ui/progress_bar.h"

static const char *TAG = "ui_config";

static uint16_t g_bar_value = 10;

static struct ui_screen_t *g_next_ui_screen = &ui_main_screen;

static ui_buttons_t buttons = {
    .top_y = 47,
    .height = 16,

    .left_x = 0,
    .right_x = 127,

    .margin_between = 2,

    .button_count = BTN_MAX,
    .buttons =
        {
            {
                .state = UI_BUTTON_STATE_ENABLED,
                .label = "\ue266",
                .forced_width = 20,
            },
            {
                .state = UI_BUTTON_STATE_ENABLED,
                .label = "\ue016",
                .forced_width = 20,
            },
            {
                .state = UI_BUTTON_STATE_ENABLED,
                .label = "\ue26e",
                .forced_width = 20,
            },
            {
                .state = UI_BUTTON_STATE_EMPTY,
            },
            {
                .state = UI_BUTTON_STATE_ENABLED,
                .label = "\ue06b",
            },
        },
};

enum ui_state_t ui_config_screen_init(u8g2_t *u8g2)
{
    u8g2_SetFontMode(u8g2, 0);
    u8g2_SetDrawColor(u8g2, 1);
    return UI_STATE_REDRAW;
}

struct ui_screen_t *ui_config_screen_deinit(void) { return g_next_ui_screen; }

enum ui_state_t ui_config_screen_draw(u8g2_t *u8g2)
{
    u8g2_SetFont(u8g2, u8g2_font_NokiaSmallPlain_te);
    u8g2_DrawStr(u8g2, 0, 10, "dupa");

    u8g2_SetFont(u8g2, u8g2_font_siji_t_6x10);
    ui_draw_buttons(u8g2, &buttons);

    return UI_STATE_IDLE;
}

enum ui_state_t ui_config_screen_process_events(void)
{
    struct btn_event_t event;
    enum ui_state_t ret = UI_STATE_REDRAW;

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
            switch (event.btn_id) {
                case BTN_1: g_next_ui_screen = &ui_set_date_screen; break;
                case BTN_2: g_next_ui_screen = &ui_set_clock_screen; break;
                case BTN_3: g_next_ui_screen = &ui_brightness_screen; break;
                case BTN_5: g_next_ui_screen = &ui_main_screen; break;
                default:
                    return UI_STATE_REDRAW;
            }

            ret = UI_STATE_DEINIT_SCREEN;
            xQueueReset(g_button_event_queue);
        }

        if (event.btn_action == BTN_ACT_UP && event.btn_id == BTN_5) {
            g_next_ui_screen = &ui_main_screen;
            ret = UI_STATE_DEINIT_SCREEN;
            xQueueReset(g_button_event_queue);
        }

        if (event.btn_action == BTN_ACT_UP && event.btn_id == BTN_3) {
            g_next_ui_screen = &ui_brightness_screen;
            ret = UI_STATE_DEINIT_SCREEN;
            xQueueReset(g_button_event_queue);
        }
    }

    return ret;
}

TickType_t ui_config_screen_wait_time(void) { return portMAX_DELAY; }

struct ui_screen_t ui_config_screen = {
    .name = "config_screen",
    .init = ui_config_screen_init,
    .deinit = ui_config_screen_deinit,
    .draw = ui_config_screen_draw,
    .process_events = ui_config_screen_process_events,
    .wait_time = ui_config_screen_wait_time,
};