#pragma once
#include "types_compat.h"
#include "file_def.h"
#include "va_list_def.h"
// #include <wchar.h>

typedef unsigned short wint_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WCHAR_MIN
#define WCHAR_MIN 0x0000
#endif

#ifndef WCHAR_MAX
#define WCHAR_MAX 0xffff
#endif

#ifndef WINT_MIN
#define WINT_MIN WCHAR_MIN
#endif

#ifndef WINT_MAX
#define WINT_MAX WCHAR_MAX
#endif

wint_t getwchar();
wint_t putwchar(wchar_t ch);
wint_t getwc(FILE *stream);
wint_t putwc(wchar_t ch, FILE *stream);
wint_t ungetwc(wint_t ch, FILE *stream);

int fwide(FILE *stream, int mode);

wint_t fgetwc(FILE *stream);
wint_t fputwc(wchar_t ch, FILE *stream);
wchar_t *fgetws(wchar_t *str, int count, FILE *stream);
int fputws(const wchar_t *str, FILE *stream);

size_t wcsftime(wchar_t *str, size_t count, const wchar_t *format, struct tm *time);

unsigned long
__wcstoul(int, int, wint_t (*wReadProc)(void *, wint_t, int), void *, int *, int *, int *);
/* __wcstoull */

long wcstol(const wchar_t *str, wchar_t **str_end, int base);
long long wcstoll(const wchar_t *str, wchar_t **str_end, int base);
unsigned long wcstoul(const wchar_t *str, wchar_t **str_end, int base);
unsigned long long wcstoull(const wchar_t *str, wchar_t **str_end, int base);

float wcstof(const wchar_t *str, wchar_t **str_end);
double wcstod(const wchar_t *str, wchar_t **str_end);
long double wcstold(const wchar_t *str, wchar_t **str_end);

wchar_t *wmemcpy(wchar_t *dest, const wchar_t *src, size_t count);
wchar_t *wmemmove(wchar_t *dest, const wchar_t *src, size_t count);
int wmemcmp(const wchar_t *lhs, const wchar_t *rhs, size_t count);
wchar_t *wmemchr(const wchar_t *ptr, wchar_t ch, size_t count);
wchar_t *wmemset(wchar_t *dest, wchar_t ch, size_t count);

int wprintf(const wchar_t *format, ...);
int fwprintf(FILE *stream, const wchar_t *format, ...);
int swprintf(wchar_t *buffer, size_t bufsz, const wchar_t *format, ...);

int vwprintf(const wchar_t *format, va_list vlist);
int vfwprintf(FILE *stream, const wchar_t *format, va_list vlist);
int vswprintf(wchar_t *buffer, size_t bufsz, const wchar_t *format, va_list vlist);

int wscanf(const wchar_t *format, ...);
int fwscanf(FILE *stream, const wchar_t *format, ...);
int swscanf(const wchar_t *buffer, const wchar_t *format, ...);

int vwscanf(const wchar_t *format, va_list vlist);
int vfwscanf(FILE *stream, const wchar_t *format, va_list vlist);
int vswscanf(const wchar_t *buffer, const wchar_t *format, va_list vlist);

size_t wcslen(const wchar_t *str);

wchar_t *wcscpy(wchar_t *dest, const wchar_t *src);
wchar_t *wcsncpy(wchar_t *dest, const wchar_t *src, size_t count);
wchar_t *wcscat(wchar_t *dest, const wchar_t *src);
wchar_t *wcsncat(wchar_t *dest, const wchar_t *src, size_t count);
size_t wcsxfrm(wchar_t *dest, const wchar_t *src, size_t count);

int wcscmp(const wchar_t *lhs, const wchar_t *rhs);
int wcsncmp(const wchar_t *lhs, const wchar_t *rhs, size_t count);
int wcscoll(const wchar_t *lhs, const wchar_t *rhs);
wchar_t *wcschr(const wchar_t *str, wchar_t ch);
wchar_t *wcsrchr(const wchar_t *str, wchar_t ch);
size_t wcsspn(const wchar_t *dest, const wchar_t *src);
size_t wcscspn(const wchar_t *dest, const wchar_t *src);

wchar_t *wcspbrk(const wchar_t *dest, const wchar_t *str);
wchar_t *wcsstr(const wchar_t *dest, const wchar_t *src);
wchar_t *wcstok(wchar_t *str, const wchar_t *delim, wchar_t **ptr);

/* TODO */
struct mbstate_t {
    int dummy;
};

wint_t btowc(int c);
int wctob(wint_t c);

/* clang-format off */
size_t mbrlen(const char * s, size_t n, struct mbstate_t * ps);
size_t mbrtowc(wchar_t * pwc, const char * s, size_t n, struct mbstate_t * ps);
int mbsinit(const struct mbstate_t *ps);
size_t mbsrtowcs(wchar_t * dst, const char ** src, size_t len, struct mbstate_t * ps);
size_t wcrtomb(char * s, wchar_t wc, struct mbstate_t * ps);
size_t wcsrtombs(char * dst, const wchar_t ** src, size_t len, struct mbstate_t * ps);
/* clang-format on */

#ifdef __cplusplus
}
#endif
