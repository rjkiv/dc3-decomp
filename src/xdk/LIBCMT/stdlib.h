#pragma once
#include "size_def.h"

#ifdef __cplusplus
extern "C" {
#endif

void *malloc(size_t size);
void *calloc(size_t nitems, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *);

typedef struct {
    int quot;
    int rem;
} div_t;

typedef struct {
    long quot;
    long rem;
} ldiv_t;

typedef struct {
    long long quot;
    long long rem;
} lldiv_t;

int abs(int n);
long labs(long n);
long long llabs(long long n);

div_t div(int x, int y);
ldiv_t ldiv(long x, long y);
lldiv_t lldiv(long long x, long long y);

void qsort(void *ptr, size_t count, size_t size, int (*comp)(const void *, const void *));
void *bsearch(
    const void *key,
    const void *ptr,
    size_t count,
    size_t size,
    int (*comp)(const void *, const void *)
);

int __mbtowc_noconv(wchar_t *, const char *, size_t);
int __wctomb_noconv(char *, wchar_t);

int mblen(const char *s, size_t n);
int mbtowc(wchar_t * pwc, const char * s, size_t n);
size_t mbstowcs(wchar_t * dst, const char * src, size_t len);
int wctomb(char *s, wchar_t wc);
size_t wcstombs(char * dst, const wchar_t * src, size_t len);

int rand();
void srand(unsigned int seed);

long double __strtold(
    int max_width,
    int (*ReadProc)(void *, int, int),
    void *ReadProcArg,
    int *chars_scanned,
    int *overflow
);

double atof(const char *str);

float strtof(const char * str, char ** str_end);
double strtod(const char * str, char ** str_end);
long double strtold(const char * str, char ** str_end);

unsigned long __strtoul(
    int base,
    int max_width,
    int (*ReadProc)(void *, int, int),
    void *ReadProcArg,
    int *chars_scanned,
    int *negative,
    int *overflow
);
unsigned long long __strtoull(
    int base,
    int max_width,
    int (*ReadProc)(void *, int, int),
    void *ReadProcArg,
    int *chars_scanned,
    int *negative,
    int *overflow
);

int atoi(const char *str);
long atol(const char *str);
long long atoll(const char *str);

long strtol(const char * str, char ** str_end, int base);
long long strtoll(const char * str, char ** str_end, int base);
unsigned long strtoul(const char * str, char ** str_end, int base);
unsigned long long strtoull(const char * str, char ** str_end, int base);

int system(const char *command);
char *getenv(const char *name);

int atexit(void (*func)(void));
void exit(int);
void abort(void);
void _Exit(int exit_code);

#ifdef __cplusplus
}
#endif
