/*
    Newclock
    tasks/button_handling.h

    Copyright (c) 2025 Rafal Macyszyn

    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <freertos/FreeRTOS.h>

enum __attribute__((packed)) btn_id_t
{
    BTN_1 = 0,
    BTN_2,
    BTN_3,
    BTN_4,
    BTN_5,
    BTN_MAX,
};

enum __attribute__((packed)) btn_action_t
{
    BTN_ACT_UP = 0,
    BTN_ACT_DOWN,
    BTN_ACT_MAX,
};

struct btn_event_t
{
    enum btn_id_t btn_id;
    enum btn_action_t btn_action;
};

extern QueueHandle_t g_button_event_queue;

const char *btn_id_t_str(const enum btn_id_t btn);
const char *btn_action_t_str(const enum btn_action_t action);

void setup_button_handler_queue(void);