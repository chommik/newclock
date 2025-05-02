/*
    Newclock
    ui/utils.c

    Copyright (c) 2025 Rafal Macyszyn

    SPDX-License-Identifier: BSD-3-Clause
*/

#include "ui.h"

const char* ui_state_str(const enum ui_state_t state) {
    switch (state) {
        case UI_STATE_DEINIT_SCREEN: return "DEINIT_SCREEN";
        case UI_STATE_IDLE: return "IDLE";
        case UI_STATE_INIT_SCREEN: return "INIT_SCREEN";
        case UI_STATE_PROCESS_EVENTS: return "PROCESS_EVENTS";
        case UI_STATE_REDRAW: return "REDRAW";
        default: return "UNKNOWN_STATE";
    }
}