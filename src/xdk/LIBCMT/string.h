#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>

u32 strlen(const char *);
u32 strcmp(const char *, const char *);
u32 stricmp(const char *, const char *);
u32 strncmp(const char *, const char *, u32);
u32 strcpy(char *, const char *);
u32 strncpy(char *, const char *, u32);

#ifdef __cplusplus
}
#endif
