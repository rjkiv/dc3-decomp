#pragma once
#include "../win_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NUIAUDIO_RESULTS { /* Size=0x10 */
    /* 0x0000 */ INT16 *pOutputMic;
    /* 0x0004 */ UINT32 Duration;
    /* 0x0008 */ FLOAT BeamAngle;
    /* 0x000c */ FLOAT Confidence;
} NUIAUDIO_RESULTS;

typedef void NUIAUDIO_ERROR_CALLBACK(HRESULT);
typedef void NUIAUDIO_CALLBACK(NUIAUDIO_RESULTS *);

HRESULT NuiAudioCreate(
    UINT32 uHardwareThreadRequested,
    NUIAUDIO_ERROR_CALLBACK *pfnErrorCallback,
    DWORD Flags,
    HANDLE Handle,
    UINT32 *pHardwareThreadUsed
);
void NuiAudioRelease(const HANDLE Handle);
void NuiAudioRegisterCallbacks(
    const HANDLE Handle, DWORD Flags, NUIAUDIO_CALLBACK *pfnProcessingCallback
);
void NuiAudioUnregisterCallbacks(
    const HANDLE Handle, NUIAUDIO_CALLBACK *pfnProcessingCallback
);

#ifdef __cplusplus
}
#endif
