#pragma once
#include "types_compat.h"
#include "time_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/* #define CLOCKS_PER_SEC */

typedef long long __time64_t;

time_t time(time_t *arg);
__time64_t _time64(__time64_t *arg);

double difftime(time_t time_end, time_t time_beg);
clock_t clock(void);

char *ctime(const time_t *timer);
char *asctime(const tm *time_ptr);
size_t strftime(char *str, size_t count, const char *format, const tm *tp);

time_t mktime(tm *arg);
tm *gmtime(const time_t *timer);
tm *_gmtime64(const __time64_t *timer);

// tm* _gmtime64(const int64_t* timp);

tm *localtime(const time_t *timer);

#ifdef __cplusplus
}
#endif
