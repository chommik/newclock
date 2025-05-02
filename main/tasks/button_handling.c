/*
    Newclock
    tasks/button_handling.c

    Copyright (c) 2025 Rafal Macyszyn

    SPDX-License-Identifier: BSD-3-Clause
*/

#include "button_handling.h"

#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <hal/gpio_types.h>

#include <stdbool.h>
#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/param.h>

#include "portmacro.h"

static const char *TAG = "btn_hdlr";

QueueHandle_t g_button_event_queue;

#define ESP_INTR_FLAG_DEFAULT 0

#define DEBOUNCE_TICKS_MIN (150 / portTICK_PERIOD_MS)

#define BUTTON_1_PIN 34
#define BUTTON_2_PIN 35
#define BUTTON_3_PIN 32
#define BUTTON_4_PIN 33
#define BUTTON_5_PIN 19

TickType_t g_last_action_tick_time[BTN_ACT_MAX][BTN_MAX] = {0};

const char *btn_id_t_str(const enum btn_id_t btn)
{
    switch (btn) {
        case BTN_1: return "BTN-1";
        case BTN_2: return "BTN-2";
        case BTN_3: return "BTN-3";
        case BTN_4: return "BTN-4";
        case BTN_5: return "BTN-5";
        default: return "BTN-UNKNOWN";
    }
}

const char *btn_action_t_str(const enum btn_action_t action)
{
    switch (action) {
        case BTN_ACT_UP: return "BTN-UP";
        case BTN_ACT_DOWN: return "BTN-DOWN";
        default: return "BTN-UNKNOWN";
    }
}

static void button_handler_isr(void *arg)
{
    uint32_t pin_number = (uint32_t)arg;
    enum btn_id_t pin_btn_mapping[] = {
        [BUTTON_1_PIN] = BTN_1,
        [BUTTON_2_PIN] = BTN_2,
        [BUTTON_3_PIN] = BTN_3,
        [BUTTON_4_PIN] = BTN_4,
        [BUTTON_5_PIN] = BTN_5,
    };
    enum btn_id_t button = pin_btn_mapping[pin_number];

    enum btn_action_t action;
    switch (gpio_get_level(pin_number)) {
        case 0: action = BTN_ACT_DOWN; break;
        case 1: action = BTN_ACT_UP; break;
        default: __unreachable(); return;
    }

    TickType_t now = xTaskGetTickCountFromISR();
    TickType_t last_event = g_last_action_tick_time[action][button];

    if ((now - last_event) < DEBOUNCE_TICKS_MIN)
        return;

    g_last_action_tick_time[action][button] = now;

    struct btn_event_t event = {
        .btn_id = button,
        .btn_action = action,
    };

    xQueueSendFromISR(g_button_event_queue, &event, NULL);
}

static void setup_button_gpio(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = ((1ULL << BUTTON_1_PIN) | (1ULL << BUTTON_2_PIN) | (1ULL << BUTTON_3_PIN) |
                         (1ULL << BUTTON_4_PIN) | (1ULL << BUTTON_5_PIN)),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    gpio_isr_handler_add(BUTTON_1_PIN, button_handler_isr, (void *)BUTTON_1_PIN);
    gpio_isr_handler_add(BUTTON_2_PIN, button_handler_isr, (void *)BUTTON_2_PIN);
    gpio_isr_handler_add(BUTTON_3_PIN, button_handler_isr, (void *)BUTTON_3_PIN);
    gpio_isr_handler_add(BUTTON_4_PIN, button_handler_isr, (void *)BUTTON_4_PIN);
    gpio_isr_handler_add(BUTTON_5_PIN, button_handler_isr, (void *)BUTTON_5_PIN);
}

void setup_button_handler_queue(void)
{
    g_button_event_queue = xQueueCreate(16, sizeof(struct btn_event_t));
    ESP_LOGD(TAG, "button handler queue created");

    setup_button_gpio();
}