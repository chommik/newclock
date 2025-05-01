#pragma once

#include <stddef.h>
#include <time.h>

int str_nreplace(char* str, size_t len, char from, char to);

int format_tm_polish(const struct tm tm_now, char* dest, size_t dest_len);