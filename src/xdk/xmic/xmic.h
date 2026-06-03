#pragma once
#include "xdk/win_types.h"
#include "xdk/xapilibi/xbase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _RMC_REQUEST { /* Size=0x4 */
    /* 0x0000 */ WORD RequestCode;
    /* 0x0002 */ WORD DeviceIndex;
} RMC_REQUEST;

struct _RMC_ASYNC_REQUEST : public _RMC_REQUEST { /* Size=0xc */
    /* 0x0000: fields for _RMC_REQUEST */
    /* 0x0004 */ void *CompletionContext;
    /* 0x0008 */ void (*CompletionRoutine)(_RMC_ASYNC_REQUEST *, INT);
} RMC_ASYNC_REQUEST;

typedef struct _XMICCAPABILITIES { /* Size=0x1c */
    /* 0x0000 */ DWORD dwFeatures;
    /* 0x0004 */ WORD wFormatTag;
    /* 0x0006 */ WORD nChannels;
    /* 0x0008 */ DWORD dwSampleRatesSupported;
    /* 0x000c */ WORD wBitsPerSample;
    /* 0x000e */ WORD dwFrameLength;
    /* 0x0010 */ BYTE bColor;
    /* 0x0012 */ WORD wVendorId;
    /* 0x0014 */ WORD wProductId;
    /* 0x0016 */ WORD wDeviceRevision;
    /* 0x0018 */ DWORD dwXid;
} XMICCAPABILITIES;

DWORD XMicGetGain(DWORD, DWORD, float *);
DWORD XMicSetGain(DWORD, float, XOVERLAPPED *);
DWORD XMicGetStatus(DWORD);
DWORD XMicRequestData(
    DWORD dwMicrophoneIndex,
    DWORD dwFrameCount,
    BYTE *pbTransferBuffer,
    WORD *pwFrameTransferLengths,
    XOVERLAPPED *pOverlapped
);

DWORD XMicGetCapabilities(DWORD dwMicrophoneIndex, XMICCAPABILITIES *pCapabilities);
DWORD XMicStart(
    DWORD dwMicrophoneIndex,
    DWORD bitSampleRate,
    DWORD *pdwFrameTransferLength,
    XOVERLAPPED *pOverlapped
);
DWORD XMicStop(DWORD dwMicrophoneIndex, XOVERLAPPED *pOverlapped);

// 0x82ec86a8: XMicGetRequestInfo
// 0x82ec8748: XMicGetDeviceInfo
// 0x82ec87c8: XMicGetDeviceType
// 0x82ec8870: XMicGetDeviceStatus
// 0x82ec8958: XMicSetupNonOverlapped
// 0x82ec89a0: XMicWaitNonOverlapped
// 0x82ec8a00: XMicResetOverlapped
// 0x82ec8a78: XMicApcRundownRoutine
// 0x82ec8ae0: XMicApcKernelRoutine
// 0x82ec8ae8: XMicApcNormalRoutine
// 0x82ec8b10: XMicConvertToBigEndian
// 0x82ec8ba0: XMicUpSample2To3

// 0x82ec8db0: XMicSetupOverlapped
// 0x82ec8e90: XMicSignalOverlapped
// 0x82ec8f58: XMicHandleRmcCompletion
// 0x82ec9088: XMicHandleMicCompletion
// 0x82ec9668: XMicHandleIsochCompletion

// 0x82ec98a0: XMicOverlappedRmcDeviceRequest
// 0x82ec9930: XMicOverlappedMicDeviceRequest

// 0x82ec9f58: XMicGetStringDescriptor
// 0x82eca018: XMicGetDeviceInformation

#ifdef __cplusplus
}
#endif

float XAudio2AmplitudeRatioToDecibels(float);
void XMicHandleDataTransferCompletion(struct _RMC_ASYNC_REQUEST *, long);
