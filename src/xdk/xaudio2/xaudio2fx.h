#pragma once
#include "xdk/xaudio2/xaudio2.h"
#include "xdk/win_types.h"

// https://learn.microsoft.com/en-us/windows/win32/api/xaudio2fx/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XAUDIO2FX_REVERB_I3DL2_PARAMETERS { /* Size=0x34 */
    float WetDryMix;
    INT32 Room;
    INT32 RoomHF;
    float RoomRolloffFactor;
    float DecayTime;
    float DecayHFRatio;
    INT32 Reflections;
    float ReflectionsDelay;
    INT32 Reverb;
    float ReverbDelay;
    float Diffusion;
    float Density;
    float HFReference;
} XAUDIO2FX_REVERB_I3DL2_PARAMETERS;

typedef struct XAUDIO2FX_REVERB_PARAMETERS { /* Size=0x38 */
    float WetDryMix;
    UINT32 ReflectionsDelay;
    BYTE ReverbDelay;
    BYTE RearDelay;
    BYTE SideDelay;
    BYTE PositionLeft;
    BYTE PositionRight;
    BYTE PositionMatrixLeft;
    BYTE PositionMatrixRight;
    BYTE EarlyDiffusion;
    BYTE LateDiffusion;
    BYTE LowEQGain;
    BYTE LowEQCutoff;
    BYTE HighEQGain;
    BYTE HighEQCutoff;
    float RoomFilterFreq;
    float RoomFilterMain;
    float RoomFilterHF;
    float ReflectionsGain;
    float ReverbGain;
    float DecayTime;
    float Density;
    float RoomSize;
    BOOL DisableLateField;
} XAUDIO2FX_REVERB_PARAMETERS;

typedef UINT32 XAUDIO2_PROCESSOR;

#define XAUDIO2_DEFAULT_PROCESSOR 0x1
#define XAUDIO2_ANY_PROCESSOR 0xFFFFFFFF

HRESULT
XAudio2Create(IXAudio2 **ppXAudio2, UINT32 Flags, XAUDIO2_PROCESSOR XAudio2Processor);

HRESULT CreateAudioReverb(IUnknown **ppApo);

inline HRESULT XAudio2CreateReverb(IUnknown **ppApo, UINT32 Flags = 0) {
    return CreateAudioReverb(ppApo);
}

#ifdef __cplusplus
}
#endif

// compiled as C++
void ReverbConvertI3DL2ToNative(
    const struct XAUDIO2FX_REVERB_I3DL2_PARAMETERS *pI3DL2,
    struct XAUDIO2FX_REVERB_PARAMETERS *pNative
);
