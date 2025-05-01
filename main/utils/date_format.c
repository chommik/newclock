#include <stdio.h>

#include "utils.h"

static const char *weekdays[] = {
    "niedziela",
    "poniedziałek",
    "wtorek",
    "środa",
    "czwartek",
    "piątek",
    "sobota",
};

static const char *months_genitive_lowercase[] = {
    "stycznia",
    "lutego",
    "marca",
    "kwietnia",
    "maja",
    "czerwca",
    "lipca",
    "sierpnia",
    "września",
    "października",
    "listopada",
    "grudnia",
};

int format_tm_polish(const struct tm tm_now, char *dest, size_t dest_len)
{
    return snprintf(dest, dest_len, "%s, %d. %s %d", weekdays[tm_now.tm_wday], tm_now.tm_mday,
        months_genitive_lowercase[tm_now.tm_mon], tm_now.tm_year + 1900);
}