#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>

void *memcpy(void *dest, const void *src, u32 count);
void memset(void *dest, u32 data, u32 count);

#ifdef __cplusplus
}
#endif
