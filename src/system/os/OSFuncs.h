#pragma once
#include "xdk/XAPILIB.h"
#include "os/ThreadCall.h"

inline DWORD CurrentThreadId() { return GetCurrentThreadId(); }

inline bool MainThread() {
    return gMainThreadID == -1 || gMainThreadID == CurrentThreadId();
}

bool ValidateThreadId(unsigned long);
