#pragma once
#include "../win_types.h"

#ifdef __cplusplus
extern "C" {
#endif

HRESULT NuiInitialize(DWORD dwFlags, DWORD dwHardwareThreadSkeleton);
void NuiShutdown();

#ifdef __cplusplus
}
#endif
