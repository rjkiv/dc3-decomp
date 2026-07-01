#pragma once
#include "types_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef long long time_t;
typedef long long __time64_t;
typedef long clock_t;

/** Structure holding a calendar date and time broken down into its components. */
typedef struct tm { /* Size=0x24 */
    /** seconds after the minute - [0, 60] including leap second */
    /* 0x0000 */ int tm_sec;
    /** minutes after the hour - [0, 59] */
    /* 0x0004 */ int tm_min;
    /** hours since midnight - [0, 23] */
    /* 0x0008 */ int tm_hour;
    /** day of the month - [1, 31] */
    /* 0x000c */ int tm_mday;
    /** months since January - [0, 11] */
    /* 0x0010 */ int tm_mon;
    /** years since 1900 */
    /* 0x0014 */ int tm_year;
    /** days since Sunday - [0, 6] */
    /* 0x0018 */ int tm_wday;
    /** days since January 1 - [0, 365] */
    /* 0x001c */ int tm_yday;
    /** daylight savings time flag */
    /* 0x0020 */ int tm_isdst;
} tm;

/* #define CLOCKS_PER_SEC */

time_t time(time_t *timeptr);
__time64_t _time64(__time64_t *timeptr);

double difftime(time_t b, time_t a);
clock_t clock();

char *ctime(const time_t *timer);
char *asctime(const tm *time_ptr);
size_t strftime(char *str, size_t count, const char *format, const tm *tp);

time_t mktime(tm *arg);
tm *gmtime(const time_t *timer);
tm *_gmtime64(const __time64_t *timer);

tm *localtime(const time_t *timer);

#ifdef __cplusplus
}
#endif
