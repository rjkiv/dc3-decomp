#pragma once
// #include <stddef.h> // we're now pulling in MSVC's built-in stddef this way
#include "types_compat.h"

#define NULL 0

#ifdef __cplusplus
extern "C" {
#endif

// #if !defined(__cplusplus)
// #include <wchar.h>
// #endif

#ifndef _PTRDIFF_T_DEFINED
typedef int ptrdiff_t;
#define _PTRDIFF_T_DEFINED
#endif

#define offsetof(T, mem) ((int)&((T *)0->mem))

#ifdef __cplusplus
}
#endif
