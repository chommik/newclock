/*
    Newclock
    utils/str_replace.c

    Copyright (c) 2025 Rafal Macyszyn

    SPDX-License-Identifier: BSD-3-Clause
*/

#include "utils.h"

int str_nreplace(char* str, size_t len, char from, char to) {
    int total = 0;
    for (size_t i = 0; i < len; i++) {
        if (str[i] == '\0')
            break;
        else if (str[i] == from) {
            str[i] = to;
            total++;
        }
    }
    return total;
}