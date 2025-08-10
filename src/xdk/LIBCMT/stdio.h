#pragma once
#include "file_def.h"
#include "va_list_def.h"

#ifdef __cplusplus
extern "C" {
#endif

int fseek(FILE *stream, unsigned int offset, int whence);
int _fseek(FILE *stream, unsigned int offset, int whence);
int ftell(FILE *stream);
int _ftell(FILE *stream);

int fgetpos(FILE *stream, fpos_t *pos);
int fsetpos(FILE *stream, const fpos_t *pos);
void rewind(FILE *stream);

FILE *fopen(const char *filename, const char *mode);
FILE *freopen(const char *filename, const char *mode, FILE *stream);
int fclose(FILE *file);

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *buffer, size_t size, size_t count, FILE *stream);
int fflush(FILE *file);

int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int sprintf(char *buffer, const char *format, ...);
int snprintf(char *buffer, size_t bufsz, const char *format, ...);

int vprintf(const char *format, va_list vlist);
int vfprintf(FILE *stream, const char *format, va_list vlist);
int vsprintf(char *buffer, const char *format, va_list vlist);
int vsnprintf(char *buffer, size_t bufsz, const char *format, va_list vlist);

int scanf(const char *format, ...);
int fscanf(FILE *stream, const char *format, ...);
int sscanf(const char *buffer, const char *format, ...);

int vscanf(const char *format, va_list vlist);
int vfscanf(FILE *stream, const char *format, va_list vlist);
int vsscanf(const char *buffer, const char *format, va_list vlist);

int getc(FILE *stream);
char *gets(char *str);
int ungetc(int ch, FILE *stream);
int putc(int ch, FILE *stream);
int puts(const char *str);
int getchar(void);
int putchar(int ch);

void setbuf(FILE *stream, char *buffer);
int setvbuf(FILE *stream, char *buffer, int mode, size_t size);

int fgetc(FILE *stream);
int fputc(int ch, FILE *stream);
char *fgets(char *str, int count, FILE *stream);
int fputs(const char *str, FILE *stream);

void clearerr(FILE *stream);
int feof(FILE *stream);
int ferror(FILE *stream);
void perror(const char *s);

int remove(const char *pathname);
int rename(const char *old_filename, const char *new_filename);
FILE *tmpfile(void);
char *tmpnam(char *filename);

#ifdef __cplusplus
}
#endif
