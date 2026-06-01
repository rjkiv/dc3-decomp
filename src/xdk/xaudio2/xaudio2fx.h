#pragma once
#include "xdk/xaudio2/xaudio2.h"
#include "xdk/win_types.h"
#include <math.h>

// https://learn.microsoft.com/en-us/windows/win32/api/xaudio2fx/
// https://github.com/DJYar/xray-oxygen/blob/master/code/SDK/include/dx/XAudio2fx.h

// All structures defined in this file should use tight packing
#pragma pack(push, 1)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XAUDIO2FX_REVERB_I3DL2_PARAMETERS { /* Size=0x34 */
    /* 0x0000 */ float WetDryMix;
    /* 0x0004 */ INT32 Room;
    /* 0x0008 */ INT32 RoomHF;
    /* 0x000c */ float RoomRolloffFactor;
    /* 0x0010 */ float DecayTime;
    /* 0x0014 */ float DecayHFRatio;
    /* 0x0018 */ INT32 Reflections;
    /* 0x001c */ float ReflectionsDelay;
    /* 0x0020 */ INT32 Reverb;
    /* 0x0024 */ float ReverbDelay;
    /* 0x0028 */ float Diffusion;
    /* 0x002c */ float Density;
    /* 0x0030 */ float HFReference;
} XAUDIO2FX_REVERB_I3DL2_PARAMETERS;

typedef struct XAUDIO2FX_REVERB_PARAMETERS { /* Size=0x38 */
    /* 0x0000 */ float WetDryMix;
    /* 0x0004 */ UINT32 ReflectionsDelay;
    /* 0x0008 */ BYTE ReverbDelay;
    /* 0x0009 */ BYTE RearDelay;
    /* 0x000a */ BYTE PositionLeft;
    /* 0x000b */ BYTE PositionRight;
    /* 0x000c */ BYTE PositionMatrixLeft;
    /* 0x000d */ BYTE PositionMatrixRight;
    /* 0x000e */ BYTE EarlyDiffusion;
    /* 0x000f */ BYTE LateDiffusion;
    /* 0x0010 */ BYTE LowEQGain;
    /* 0x0011 */ BYTE LowEQCutoff;
    /* 0x0012 */ BYTE HighEQGain;
    /* 0x0013 */ BYTE HighEQCutoff;
    /* 0x0014 */ float RoomFilterFreq;
    /* 0x0018 */ float RoomFilterMain;
    /* 0x001c */ float RoomFilterHF;
    /* 0x0020 */ float ReflectionsGain;
    /* 0x0024 */ float ReverbGain;
    /* 0x0028 */ float DecayTime;
    /* 0x002c */ float Density;
    /* 0x0030 */ float RoomSize;
    /* 0x0034 */ BOOL DisableLateField;
} XAUDIO2FX_REVERB_PARAMETERS;

// Maximum, minimum and default values for the parameters above
#define XAUDIO2FX_REVERB_MIN_WET_DRY_MIX 0.0f
#define XAUDIO2FX_REVERB_MIN_REFLECTIONS_DELAY 0
#define XAUDIO2FX_REVERB_MIN_REVERB_DELAY 0
#define XAUDIO2FX_REVERB_MIN_REAR_DELAY 0
#define XAUDIO2FX_REVERB_MIN_POSITION 0
#define XAUDIO2FX_REVERB_MIN_DIFFUSION 0
#define XAUDIO2FX_REVERB_MIN_LOW_EQ_GAIN 0
#define XAUDIO2FX_REVERB_MIN_LOW_EQ_CUTOFF 0
#define XAUDIO2FX_REVERB_MIN_HIGH_EQ_GAIN 0
#define XAUDIO2FX_REVERB_MIN_HIGH_EQ_CUTOFF 0
#define XAUDIO2FX_REVERB_MIN_ROOM_FILTER_FREQ 20.0f
#define XAUDIO2FX_REVERB_MIN_ROOM_FILTER_MAIN -100.0f
#define XAUDIO2FX_REVERB_MIN_ROOM_FILTER_HF -100.0f
#define XAUDIO2FX_REVERB_MIN_REFLECTIONS_GAIN -100.0f
#define XAUDIO2FX_REVERB_MIN_REVERB_GAIN -100.0f
#define XAUDIO2FX_REVERB_MIN_DECAY_TIME 0.1f
#define XAUDIO2FX_REVERB_MIN_DENSITY 0.0f
#define XAUDIO2FX_REVERB_MIN_ROOM_SIZE 0.0f

#define XAUDIO2FX_REVERB_MAX_WET_DRY_MIX 100.0f
#define XAUDIO2FX_REVERB_MAX_REFLECTIONS_DELAY 300
#define XAUDIO2FX_REVERB_MAX_REVERB_DELAY 85
#define XAUDIO2FX_REVERB_MAX_REAR_DELAY 5
#define XAUDIO2FX_REVERB_MAX_POSITION 30
#define XAUDIO2FX_REVERB_MAX_DIFFUSION 15
#define XAUDIO2FX_REVERB_MAX_LOW_EQ_GAIN 12
#define XAUDIO2FX_REVERB_MAX_LOW_EQ_CUTOFF 9
#define XAUDIO2FX_REVERB_MAX_HIGH_EQ_GAIN 8
#define XAUDIO2FX_REVERB_MAX_HIGH_EQ_CUTOFF 14
#define XAUDIO2FX_REVERB_MAX_ROOM_FILTER_FREQ 20000.0f
#define XAUDIO2FX_REVERB_MAX_ROOM_FILTER_MAIN 0.0f
#define XAUDIO2FX_REVERB_MAX_ROOM_FILTER_HF 0.0f
#define XAUDIO2FX_REVERB_MAX_REFLECTIONS_GAIN 20.0f
#define XAUDIO2FX_REVERB_MAX_REVERB_GAIN 20.0f
#define XAUDIO2FX_REVERB_MAX_DENSITY 100.0f
#define XAUDIO2FX_REVERB_MAX_ROOM_SIZE 100.0f

#define XAUDIO2FX_REVERB_DEFAULT_WET_DRY_MIX 100.0f
#define XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_DELAY 5
#define XAUDIO2FX_REVERB_DEFAULT_REVERB_DELAY 5
#define XAUDIO2FX_REVERB_DEFAULT_REAR_DELAY 5
#define XAUDIO2FX_REVERB_DEFAULT_POSITION 6
#define XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX 27
#define XAUDIO2FX_REVERB_DEFAULT_EARLY_DIFFUSION 8
#define XAUDIO2FX_REVERB_DEFAULT_LATE_DIFFUSION 8
#define XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_GAIN 8
#define XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_CUTOFF 4
#define XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_GAIN 8
#define XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_CUTOFF 4
#define XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_FREQ 5000.0f
#define XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_MAIN 0.0f
#define XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_HF 0.0f
#define XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_GAIN 0.0f
#define XAUDIO2FX_REVERB_DEFAULT_REVERB_GAIN 0.0f
#define XAUDIO2FX_REVERB_DEFAULT_DECAY_TIME 1.0f
#define XAUDIO2FX_REVERB_DEFAULT_DENSITY 100.0f
#define XAUDIO2FX_REVERB_DEFAULT_ROOM_SIZE 100.0f

// clang-format off
#define XAUDIO2FX_I3DL2_PRESET_DEFAULT         {100,-10000,    0,0.0f, 1.00f,0.50f,-10000,0.020f,-10000,0.040f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_GENERIC         {100, -1000, -100,0.0f, 1.49f,0.83f, -2602,0.007f,   200,0.011f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_PADDEDCELL      {100, -1000,-6000,0.0f, 0.17f,0.10f, -1204,0.001f,   207,0.002f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_ROOM            {100, -1000, -454,0.0f, 0.40f,0.83f, -1646,0.002f,    53,0.003f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_BATHROOM        {100, -1000,-1200,0.0f, 1.49f,0.54f,  -370,0.007f,  1030,0.011f,100.0f, 60.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_LIVINGROOM      {100, -1000,-6000,0.0f, 0.50f,0.10f, -1376,0.003f, -1104,0.004f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_STONEROOM       {100, -1000, -300,0.0f, 2.31f,0.64f,  -711,0.012f,    83,0.017f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_AUDITORIUM      {100, -1000, -476,0.0f, 4.32f,0.59f,  -789,0.020f,  -289,0.030f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_CONCERTHALL     {100, -1000, -500,0.0f, 3.92f,0.70f, -1230,0.020f,    -2,0.029f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_CAVE            {100, -1000,    0,0.0f, 2.91f,1.30f,  -602,0.015f,  -302,0.022f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_ARENA           {100, -1000, -698,0.0f, 7.24f,0.33f, -1166,0.020f,    16,0.030f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_HANGAR          {100, -1000,-1000,0.0f,10.05f,0.23f,  -602,0.020f,   198,0.030f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_CARPETEDHALLWAY {100, -1000,-4000,0.0f, 0.30f,0.10f, -1831,0.002f, -1630,0.030f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_HALLWAY         {100, -1000, -300,0.0f, 1.49f,0.59f, -1219,0.007f,   441,0.011f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_STONECORRIDOR   {100, -1000, -237,0.0f, 2.70f,0.79f, -1214,0.013f,   395,0.020f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_ALLEY           {100, -1000, -270,0.0f, 1.49f,0.86f, -1204,0.007f,    -4,0.011f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_FOREST          {100, -1000,-3300,0.0f, 1.49f,0.54f, -2560,0.162f,  -613,0.088f, 79.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_CITY            {100, -1000, -800,0.0f, 1.49f,0.67f, -2273,0.007f, -2217,0.011f, 50.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_MOUNTAINS       {100, -1000,-2500,0.0f, 1.49f,0.21f, -2780,0.300f, -2014,0.100f, 27.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_QUARRY          {100, -1000,-1000,0.0f, 1.49f,0.83f,-10000,0.061f,   500,0.025f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_PLAIN           {100, -1000,-2000,0.0f, 1.49f,0.50f, -2466,0.179f, -2514,0.100f, 21.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_PARKINGLOT      {100, -1000,    0,0.0f, 1.65f,1.50f, -1363,0.008f, -1153,0.012f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_SEWERPIPE       {100, -1000,-1000,0.0f, 2.81f,0.14f,   429,0.014f,   648,0.021f, 80.0f, 60.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_UNDERWATER      {100, -1000,-4000,0.0f, 1.49f,0.10f,  -449,0.007f,  1700,0.011f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_SMALLROOM       {100, -1000, -600,0.0f, 1.10f,0.83f,  -400,0.005f,   500,0.010f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_MEDIUMROOM      {100, -1000, -600,0.0f, 1.30f,0.83f, -1000,0.010f,  -200,0.020f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_LARGEROOM       {100, -1000, -600,0.0f, 1.50f,0.83f, -1600,0.020f, -1000,0.040f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_MEDIUMHALL      {100, -1000, -600,0.0f, 1.80f,0.70f, -1300,0.015f,  -800,0.030f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_LARGEHALL       {100, -1000, -600,0.0f, 1.80f,0.70f, -2000,0.030f, -1400,0.060f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_PLATE           {100, -1000, -200,0.0f, 1.30f,0.90f,     0,0.002f,     0,0.010f,100.0f, 75.0f,5000.0f}
// clang-format on

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
inline void ReverbConvertI3DL2ToNative(
    const struct XAUDIO2FX_REVERB_I3DL2_PARAMETERS *pI3DL2,
    struct XAUDIO2FX_REVERB_PARAMETERS *pNative
) {
    float reflectionsDelay;
    float reverbDelay;

    // RoomRolloffFactor is ignored

    // These parameters have no equivalent in I3DL2
    pNative->RearDelay = XAUDIO2FX_REVERB_DEFAULT_REAR_DELAY; // 5
    pNative->PositionLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION; // 6
    pNative->PositionRight = XAUDIO2FX_REVERB_DEFAULT_POSITION; // 6
    pNative->PositionMatrixLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX; // 27
    pNative->PositionMatrixRight = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX; // 27
    pNative->RoomSize = XAUDIO2FX_REVERB_DEFAULT_ROOM_SIZE; // 100
    pNative->LowEQCutoff = 4;
    pNative->HighEQCutoff = 6;

    // The rest of the I3DL2 parameters map to the native property set
    pNative->RoomFilterMain = (float)pI3DL2->Room / 100.0f;
    pNative->RoomFilterHF = (float)pI3DL2->RoomHF / 100.0f;

    if (pI3DL2->DecayHFRatio >= 1.0f) {
        INT32 index = (INT32)(-4.0 * log10f(pI3DL2->DecayHFRatio));
        if (index < -8)
            index = -8;
        pNative->LowEQGain = (BYTE)((index < 0) ? index + 8 : 8);
        pNative->HighEQGain = 8;
        pNative->DecayTime = pI3DL2->DecayTime * pI3DL2->DecayHFRatio;
    } else {
        INT32 index = (INT32)(4.0 * log10f(pI3DL2->DecayHFRatio));
        if (index < -8)
            index = -8;
        pNative->LowEQGain = 8;
        pNative->HighEQGain = (BYTE)((index < 0) ? index + 8 : 8);
        pNative->DecayTime = pI3DL2->DecayTime;
    }

    reflectionsDelay = pI3DL2->ReflectionsDelay * 1000.0f;
    if (reflectionsDelay >= XAUDIO2FX_REVERB_MAX_REFLECTIONS_DELAY) // 300
    {
        reflectionsDelay = (float)(XAUDIO2FX_REVERB_MAX_REFLECTIONS_DELAY - 1);
    } else if (reflectionsDelay <= 1) {
        reflectionsDelay = 1;
    }
    pNative->ReflectionsDelay = (UINT32)reflectionsDelay;

    reverbDelay = pI3DL2->ReverbDelay * 1000.0f;
    if (reverbDelay >= XAUDIO2FX_REVERB_MAX_REVERB_DELAY) // 85
    {
        reverbDelay = (float)(XAUDIO2FX_REVERB_MAX_REVERB_DELAY - 1);
    }
    pNative->ReverbDelay = (BYTE)reverbDelay;

    pNative->ReflectionsGain = pI3DL2->Reflections / 100.0f;
    pNative->ReverbGain = pI3DL2->Reverb / 100.0f;
    pNative->EarlyDiffusion = (BYTE)(15.0f * pI3DL2->Diffusion / 100.0f);
    pNative->LateDiffusion = pNative->EarlyDiffusion;
    pNative->Density = pI3DL2->Density;
    pNative->RoomFilterFreq = pI3DL2->HFReference;

    pNative->WetDryMix = pI3DL2->WetDryMix;
    pNative->DisableLateField = false;
}

// Undo the #pragma pack(push, 1) at the top of this file
#pragma pack(pop)
