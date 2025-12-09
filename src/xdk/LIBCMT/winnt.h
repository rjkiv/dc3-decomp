#pragma once

#include "xdk/win_types.h"
#define STATUS_WAIT_0 0x00000000L

#define LANG_NEUTRAL 0x00

struct RTL_CRITICAL_SECTION {
    union {
        ULONG_PTR RawEvent[4];

    } Synchronization;

    LONG LockCount;
    LONG RecursionCount;
    HANDLE OwningThread;
};
