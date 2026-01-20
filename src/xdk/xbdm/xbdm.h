#pragma once
#include "../win_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// size 0x8
struct _DM_VERSION_INFO {
    unsigned short Major;
    unsigned short Minor;
    unsigned short Build;
    unsigned short Qfe;
};

// size 0x20
struct _DM_SYSTEM_INFO {
    int SizeOfStruct; // 0x0
    _DM_VERSION_INFO BaseKernelVersion; // 0x4
    _DM_VERSION_INFO KernelVersion; // 0xc
    _DM_VERSION_INFO XDKVersion; // 0x14
    unsigned int dmSystemInfoFlags; // 0x1c
};

typedef _DM_SYSTEM_INFO DM_SYSTEM_INFO;

HRESULT DmGetSystemInfo(DM_SYSTEM_INFO *);
HRESULT DmIsDebuggerPresent();
HRESULT DmCaptureStackBackTrace(ULONG, VOID *);
HRESULT DmMapDevkitDrive();

#ifdef __cplusplus
}
#endif
