#pragma once
#include "xdk/win_types.h"

typedef struct _GUID { /* Size=0x10 */
    /* 0x0000 */ DWORD Data1;
    /* 0x0004 */ WORD Data2;
    /* 0x0006 */ WORD Data3;
    /* 0x0008 */ BYTE Data4[8];
} GUID;

#ifdef __cplusplus
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8)                     \
    extern "C" const GUID name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }
#else
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8)                     \
    const GUID name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }
#endif
