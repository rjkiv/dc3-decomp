#pragma once
#include "../win_types.h"
#include "minwinbase.h"
#include "wtypesbase.h"

#ifdef __cplusplus
extern "C" {
#endif

#define INFINITE 0xFFFFFFFF
#define WAIT_OBJECT_0 ((STATUS_WAIT_0) + 0)

#define FORMAT_MESSAGE_FROM_SYSTEM 0x00001000

typedef struct _MEMORYSTATUS {
    DWORD dwLength;
    DWORD dwMemoryLoad;
    SIZE_T dwTotalPhys;
    SIZE_T dwAvailPhys;
    SIZE_T dwTotalPageFile;
    SIZE_T dwAvailPageFile;
    SIZE_T dwTotalVirtual;
    SIZE_T dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;

HANDLE CreateSemaphoreA(
    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    LONG lInitialCount,
    LONG lMaximumCount,
    LPCSTR lpName
);
VOID GlobalMemoryStatus(LPMEMORYSTATUS lpBuffer);

#ifdef __cplusplus
}
#endif
