#include <esp_log.h>

#include <u8g2.h>

#include "button_handling.h"
#include "ui.h"

static const char *TAG = "ui_SCREEN_NAME";

enum ui_state_t ui_SCREEN_NAME_init(u8g2_t *u8g2) { return UI_STATE_REDRAW; }

struct ui_screen_t *ui_SCREEN_NAME_deinit(void) { return &ui_main_screen; }

enum ui_state_t ui_SCREEN_NAME_draw(u8g2_t *u8g2) { return UI_STATE_IDLE; }

enum ui_state_t ui_SCREEN_NAME_process_events(void)
{
    struct btn_event_t event;
    enum ui_state_t ret = UI_STATE_REDRAW;

    while (xQueueReceive(g_button_event_queue, &event, 0) == pdPASS) {
        ESP_LOGI(TAG, "event: %s %s", btn_action_t_str(event.btn_action), btn_id_t_str(event.btn_id));

        if (event.btn_action == BTN_ACT_DOWN && event.btn_id == BTN_5) {
            ret = UI_STATE_DEINIT_SCREEN;
        }
    }

    return ret;
}

TickType_t ui_SCREEN_NAME_wait_time(void) { return 1000 / portTICK_PERIOD_MS; }

struct ui_screen_t ui_SCREEN_NAME = {
    .name = "SCREEN_NAME",
    .init = ui_SCREEN_NAME_init,
    .deinit = ui_SCREEN_NAME_deinit,
    .draw = ui_SCREEN_NAME_draw,
    .process_events = ui_SCREEN_NAME_process_events,
    .wait_time = ui_SCREEN_NAME_wait_time,
};