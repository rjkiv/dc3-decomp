#pragma once
#include "types_compat.h"
#include "time_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/* #define CLOCKS_PER_SEC */

time_t time(time_t *arg);
double difftime(time_t time_end, time_t time_beg);
clock_t clock(void);

char *ctime(const time_t *timer);
char *asctime(const struct tm *time_ptr);
size_t strftime(char *str, size_t count, const char *format, const struct tm *tp);

time_t mktime(struct tm *arg);
struct tm *gmtime(const time_t *timer);
struct tm *localtime(const time_t *timer);

#ifdef __cplusplus
}
#endif
