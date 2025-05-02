/*
    Newclock
    utils/utils.h

    Copyright (c) 2025 Rafal Macyszyn

    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <stddef.h>
#include <time.h>

int str_nreplace(char *str, size_t len, char from, char to);

int format_tm_polish(const struct tm tm_now, char *dest, size_t dest_len);