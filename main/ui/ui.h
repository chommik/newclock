#pragma once

#include <freertos/FreeRTOS.h>

#include <u8g2.h>

enum ui_state_t
{
    UI_STATE_INIT_SCREEN,
    UI_STATE_IDLE,
    UI_STATE_REDRAW,
    UI_STATE_PROCESS_EVENTS,
    UI_STATE_DEINIT_SCREEN,
};

const char* ui_state_str(const enum ui_state_t state);

typedef enum ui_state_t ui_process_events_function_t(void);
typedef TickType_t ui_wait_time_function_t(void);
typedef enum ui_state_t ui_draw_function_t(u8g2_t*);
typedef enum ui_state_t ui_init_function_t(u8g2_t*);
typedef struct ui_screen_t* ui_deinit_function_t(void);

struct ui_screen_t {
    const char* name;
    ui_init_function_t *init;
    ui_deinit_function_t *deinit;
    ui_process_events_function_t *process_events;
    ui_wait_time_function_t *wait_time;
    ui_draw_function_t *draw;
};

extern struct ui_screen_t ui_main_screen;
extern struct ui_screen_t ui_config_screen;
extern struct ui_screen_t ui_brightness_screen;
extern struct ui_screen_t ui_set_clock_screen;
extern struct ui_screen_t ui_set_date_screen;