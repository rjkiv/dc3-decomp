#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>

typedef u32 size_t;
typedef size_t ptrdiff_t;
#define offsetof(T, mem) ((int)&((T *)0->mem))

#ifdef __cplusplus
}
#endif
