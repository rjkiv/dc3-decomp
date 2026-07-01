#pragma once
#include "xdk/win_types.h"

#define MAKEWORD(a, b)                                                                   \
    ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff))                                            \
            | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))

#define HIBYTE(w) ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))
#define LOBYTE(w) ((BYTE)((w) & 0xff))
