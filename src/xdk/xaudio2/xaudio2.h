#pragma once
#include "xdk/win_types.h"
#include "xdk/unknwn.h"
#include "xdk/xapilibi/xbase.h"
#include "xapobase.h"

// https://learn.microsoft.com/en-us/windows/win32/api/xaudio2/

// All structures defined in this file use tight field packing
#pragma pack(push, 1)

struct XAUDIO2_BUFFER { /* Size=0x24 */
    /* 0x0000 */ UINT32 Flags;
    /* 0x0004 */ UINT32 AudioBytes;
    /* 0x0008 */ const BYTE *pAudioData;
    /* 0x000c */ UINT32 PlayBegin;
    /* 0x0010 */ UINT32 PlayLength;
    /* 0x0014 */ UINT32 LoopBegin;
    /* 0x0018 */ UINT32 LoopLength;
    /* 0x001c */ UINT32 LoopCount;
    /* 0x0020 */ void *pContext;
};

struct XMA2WAVEFORMATEX { /* Size=0x34 */
    /* 0x0000 */ tWAVEFORMATEX wfx;
    /* 0x0012 */ WORD NumStreams;
    /* 0x0014 */ DWORD ChannelMask;
    /* 0x0018 */ DWORD SamplesEncoded;
    /* 0x001c */ DWORD BytesPerBlock;
    /* 0x0020 */ DWORD PlayBegin;
    /* 0x0024 */ DWORD PlayLength;
    /* 0x0028 */ DWORD LoopBegin;
    /* 0x002c */ DWORD LoopLength;
    /* 0x0030 */ BYTE LoopCount;
    /* 0x0031 */ BYTE EncoderVersion;
    /* 0x0032 */ WORD BlockCount;
};

struct XAUDIO2_VOICE_DETAILS { /* Size=0xc */
    /* 0x0000 */ UINT32 CreationFlags;
    /* 0x0004 */ UINT32 InputChannels;
    /* 0x0008 */ UINT32 InputSampleRate;
};

struct XAUDIO2_EFFECT_DESCRIPTOR { /* Size=0xc */
    /* 0x0000 */ IUnknown *pEffect;
    /* 0x0004 */ BOOL InitialState;
    /* 0x0008 */ UINT32 OutputChannels;
};

struct XAUDIO2_EFFECT_CHAIN { /* Size=0x8 */
    /* 0x0000 */ UINT32 EffectCount;
    /* 0x0004 */ XAUDIO2_EFFECT_DESCRIPTOR *pEffectDescriptors;
};

struct XAUDIO2_FILTER_PARAMETERS { /* Size=0xc */
    /* 0x0000 */ XAUDIO2_FILTER_TYPE Type;
    /* 0x0004 */ float Frequency;
    /* 0x0008 */ float OneOverQ;
};

struct XAUDIO2_SEND_DESCRIPTOR { /* Size=0x8 */
    /* 0x0000 */ UINT32 Flags;
    /* 0x0004 */ struct IXAudio2Voice *pOutputVoice;
};

// clang-format off
struct XAUDIO2_VOICE_SENDS { /* Size=0x8 */
    /* 0x0000 */ UINT32 SendCount;
    /* 0x0004 */ XAUDIO2_SEND_DESCRIPTOR *pSends;
};
// clang-format on

struct NUI_TALKER_POSITION { /* Size=0x8 */
    /* 0x0000 */ float fDirection;
    /* 0x0004 */ float fConfidence;
};

// https://learn.microsoft.com/en-us/windows/win32/api/xaudio2/nn-xaudio2-ixaudio2voice
struct IXAudio2Voice { /* Size=0x4 */
    virtual void GetVoiceDetails(XAUDIO2_VOICE_DETAILS *pVoiceDetails);
    virtual HRESULT SetOutputVoices(const XAUDIO2_VOICE_SENDS *pSendList);
    virtual HRESULT SetEffectChain(const XAUDIO2_EFFECT_CHAIN *pEffectChain);
    virtual HRESULT EnableEffect(UINT32 EffectIndex, UINT32 OperationSet);
    virtual HRESULT DisableEffect(UINT32 EffectIndex, UINT32 OperationSet);
    virtual void GetEffectState(UINT32 EffectIndex, BOOL *pfEnabled);
    virtual HRESULT SetEffectParameters(
        UINT32 EffectIndex,
        const void *pParameters,
        UINT32 ParametersByteSize,
        UINT32 OperationSet
    );
    virtual HRESULT
    GetEffectParameters(UINT32 EffectIndex, void *pParameters, UINT32 ParametersByteSize);
    virtual HRESULT
    SetFilterParameters(const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet);
    virtual void GetFilterParameters(XAUDIO2_FILTER_PARAMETERS *pParameters);
    virtual HRESULT SetOutputFilterParameters(
        IXAudio2Voice *pDestinationVoice,
        const XAUDIO2_FILTER_PARAMETERS *pParameters,
        UINT32 OperationSet
    );
    virtual void GetOutputFilterParameters(
        IXAudio2Voice *pDestinationVoice, XAUDIO2_FILTER_PARAMETERS *pParameters
    );
    virtual HRESULT SetVolume(float Volume, UINT32 OperationSet);
    virtual void GetVolume(float *pVolume);
    virtual HRESULT
    SetChannelVolumes(UINT32 Channels, const float *pVolumes, UINT32 OperationSet);
    virtual void GetChannelVolumes(UINT32 Channels, float *pVolumes);
    virtual HRESULT SetOutputMatrix(
        IXAudio2Voice *pDestinationVoice,
        UINT32 SourceChannels,
        UINT32 DestinationChannels,
        const float *pLevelMatrix,
        UINT32 OperationSet
    );
    virtual void GetOutputMatrix(
        IXAudio2Voice *pDestinationVoice,
        UINT32 SourceChannels,
        UINT32 DestinationChannels,
        float *pLevelMatrix
    );
    virtual void DestroyVoice();
    IXAudio2Voice(const IXAudio2Voice &);
    IXAudio2Voice();
    IXAudio2Voice &operator=(const IXAudio2Voice &);
};

struct IXAudio2SubmixVoice : public IXAudio2Voice { /* Size=0x4 */
    /* 0x0000: fields for IXAudio2Voice */
    virtual void GetVoiceDetails(XAUDIO2_VOICE_DETAILS *pVoiceDetails) = 0;
    virtual HRESULT SetOutputVoices(const XAUDIO2_VOICE_SENDS *pSendList) = 0;
    virtual HRESULT SetEffectChain(const XAUDIO2_EFFECT_CHAIN *pEffectChain) = 0;
    virtual HRESULT EnableEffect(UINT32 EffectIndex, UINT32 OperationSet) = 0;
    virtual HRESULT DisableEffect(UINT32 EffectIndex, UINT32 OperationSet) = 0;
    virtual void GetEffectState(UINT32 EffectIndex, BOOL *pfEnabled) = 0;
    virtual HRESULT SetEffectParameters(
        UINT32 EffectIndex,
        const void *pParameters,
        UINT32 ParametersByteSize,
        UINT32 OperationSet
    ) = 0;
    virtual HRESULT GetEffectParameters(
        UINT32 EffectIndex, void *pParameters, UINT32 ParametersByteSize
    ) = 0;
    virtual HRESULT SetFilterParameters(
        const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet
    ) = 0;
    virtual void GetFilterParameters(XAUDIO2_FILTER_PARAMETERS *pParameters) = 0;
    virtual HRESULT SetOutputFilterParameters(
        IXAudio2Voice *pDestinationVoice,
        const XAUDIO2_FILTER_PARAMETERS *pParameters,
        UINT32 OperationSet
    ) = 0;
    virtual void GetOutputFilterParameters(
        IXAudio2Voice *pDestinationVoice, XAUDIO2_FILTER_PARAMETERS *pParameters
    ) = 0;
    virtual HRESULT SetVolume(float Volume, UINT32 OperationSet) = 0;
    virtual void GetVolume(float *pVolume) = 0;
    virtual HRESULT
    SetChannelVolumes(UINT32 Channels, const float *pVolumes, UINT32 OperationSet) = 0;
    virtual void GetChannelVolumes(UINT32 Channels, float *pVolumes) = 0;
    virtual HRESULT SetOutputMatrix(
        IXAudio2Voice *pDestinationVoice,
        UINT32 SourceChannels,
        UINT32 DestinationChannels,
        const float *pLevelMatrix,
        UINT32 OperationSet
    ) = 0;
    virtual void GetOutputMatrix(
        IXAudio2Voice *pDestinationVoice,
        UINT32 SourceChannels,
        UINT32 DestinationChannels,
        float *pLevelMatrix
    ) = 0;
    virtual void DestroyVoice() = 0;
    IXAudio2SubmixVoice(const IXAudio2SubmixVoice &);
    IXAudio2SubmixVoice();
    IXAudio2SubmixVoice &operator=(const IXAudio2SubmixVoice &);
};

struct XAUDIO2_BUFFER_WMA { /* Size=0x8 */
    /* 0x0000 */ const UINT32 *pDecodedPacketCumulativeBytes;
    /* 0x0004 */ UINT32 PacketCount;
};

struct XAUDIO2_VOICE_STATE { /* Size=0x10 */
    /* 0x0000 */ void *pCurrentBufferContext;
    /* 0x0004 */ UINT32 BuffersQueued;
    /* 0x0008 */ UINT64 SamplesPlayed;
};

struct IXAudio2SourceVoice : public IXAudio2Voice { /* Size=0x4 */
    /* 0x0000: fields for IXAudio2Voice */
    virtual void GetVoiceDetails(XAUDIO2_VOICE_DETAILS *pVoiceDetails) = 0;
    virtual HRESULT SetOutputVoices(const XAUDIO2_VOICE_SENDS *pSendList) = 0;
    virtual HRESULT SetEffectChain(const XAUDIO2_EFFECT_CHAIN *pEffectChain) = 0;
    virtual HRESULT EnableEffect(UINT32 EffectIndex, UINT32 OperationSet) = 0;
    virtual HRESULT DisableEffect(UINT32 EffectIndex, UINT32 OperationSet) = 0;
    virtual void GetEffectState(UINT32 EffectIndex, BOOL *pfEnabled) = 0;
    virtual HRESULT SetEffectParameters(
        UINT32 EffectIndex,
        const void *pParameters,
        UINT32 ParametersByteSize,
        UINT32 OperationSet
    ) = 0;
    virtual HRESULT GetEffectParameters(
        UINT32 EffectIndex, void *pParameters, UINT32 ParametersByteSize
    ) = 0;
    virtual HRESULT SetFilterParameters(
        const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet
    ) = 0;
    virtual void GetFilterParameters(XAUDIO2_FILTER_PARAMETERS *pParameters) = 0;
    virtual HRESULT SetOutputFilterParameters(
        IXAudio2Voice *pDestinationVoice,
        const XAUDIO2_FILTER_PARAMETERS *pParameters,
        UINT32 OperationSet
    ) = 0;
    virtual void GetOutputFilterParameters(
        IXAudio2Voice *pDestinationVoice, XAUDIO2_FILTER_PARAMETERS *pParameters
    ) = 0;
    virtual HRESULT SetVolume(float Volume, UINT32 OperationSet) = 0;
    virtual void GetVolume(float *pVolume) = 0;
    virtual HRESULT
    SetChannelVolumes(UINT32 Channels, const float *pVolumes, UINT32 OperationSet) = 0;
    virtual void GetChannelVolumes(UINT32 Channels, float *pVolumes) = 0;
    virtual HRESULT SetOutputMatrix(
        IXAudio2Voice *pDestinationVoice,
        UINT32 SourceChannels,
        UINT32 DestinationChannels,
        const float *pLevelMatrix,
        UINT32 OperationSet
    ) = 0;
    virtual void GetOutputMatrix(
        IXAudio2Voice *pDestinationVoice,
        UINT32 SourceChannels,
        UINT32 DestinationChannels,
        float *pLevelMatrix
    ) = 0;
    virtual void DestroyVoice() = 0;
    virtual HRESULT Start(UINT32, UINT32);
    virtual HRESULT Stop(UINT32, UINT32);
    virtual HRESULT
    SubmitSourceBuffer(const XAUDIO2_BUFFER *, const XAUDIO2_BUFFER_WMA *);
    virtual HRESULT FlushSourceBuffers();
    virtual HRESULT Discontinuity();
    virtual HRESULT ExitLoop(UINT32);
    virtual void GetState(XAUDIO2_VOICE_STATE *, UINT);
    virtual HRESULT SetFrequencyRatio(float, UINT32);
    virtual void GetFrequencyRatio(float *);
    virtual HRESULT SetSourceSampleRate(UINT32);
    virtual HRESULT SetMusicVoice(INT);

    IXAudio2SourceVoice(const IXAudio2SourceVoice &);
    IXAudio2SourceVoice();
    IXAudio2SourceVoice &operator=(const IXAudio2SourceVoice &);
};

struct IXAudio2MasteringVoice : public IXAudio2Voice { /* Size=0x4 */
    /* 0x0000: fields for IXAudio2Voice */
    virtual void GetVoiceDetails(XAUDIO2_VOICE_DETAILS *pVoiceDetails) = 0;
    virtual HRESULT SetOutputVoices(const XAUDIO2_VOICE_SENDS *pSendList) = 0;
    virtual HRESULT SetEffectChain(const XAUDIO2_EFFECT_CHAIN *pEffectChain) = 0;
    virtual HRESULT EnableEffect(UINT32 EffectIndex, UINT32 OperationSet) = 0;
    virtual HRESULT DisableEffect(UINT32 EffectIndex, UINT32 OperationSet) = 0;
    virtual void GetEffectState(UINT32 EffectIndex, BOOL *pfEnabled) = 0;
    virtual HRESULT SetEffectParameters(
        UINT32 EffectIndex,
        const void *pParameters,
        UINT32 ParametersByteSize,
        UINT32 OperationSet
    ) = 0;
    virtual HRESULT GetEffectParameters(
        UINT32 EffectIndex, void *pParameters, UINT32 ParametersByteSize
    ) = 0;
    virtual HRESULT SetFilterParameters(
        const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet
    ) = 0;
    virtual void GetFilterParameters(XAUDIO2_FILTER_PARAMETERS *pParameters) = 0;
    virtual HRESULT SetOutputFilterParameters(
        IXAudio2Voice *pDestinationVoice,
        const XAUDIO2_FILTER_PARAMETERS *pParameters,
        UINT32 OperationSet
    ) = 0;
    virtual void GetOutputFilterParameters(
        IXAudio2Voice *pDestinationVoice, XAUDIO2_FILTER_PARAMETERS *pParameters
    ) = 0;
    virtual HRESULT SetVolume(float Volume, UINT32 OperationSet) = 0;
    virtual void GetVolume(float *pVolume) = 0;
    virtual HRESULT
    SetChannelVolumes(UINT32 Channels, const float *pVolumes, UINT32 OperationSet) = 0;
    virtual void GetChannelVolumes(UINT32 Channels, float *pVolumes) = 0;
    virtual HRESULT SetOutputMatrix(
        IXAudio2Voice *pDestinationVoice,
        UINT32 SourceChannels,
        UINT32 DestinationChannels,
        const float *pLevelMatrix,
        UINT32 OperationSet
    ) = 0;
    virtual void GetOutputMatrix(
        IXAudio2Voice *pDestinationVoice,
        UINT32 SourceChannels,
        UINT32 DestinationChannels,
        float *pLevelMatrix
    ) = 0;
    virtual void DestroyVoice() = 0;
    IXAudio2MasteringVoice(const IXAudio2MasteringVoice &);
    IXAudio2MasteringVoice();
    IXAudio2MasteringVoice &operator=(const IXAudio2MasteringVoice &);
};

struct IXAudio2EngineCallback { /* Size=0x4 */
    virtual void OnProcessingPassStart();
    virtual void OnProcessingPassEnd();
    virtual void OnCriticalError(HRESULT Error);
    IXAudio2EngineCallback(const IXAudio2EngineCallback &);
    IXAudio2EngineCallback();
    IXAudio2EngineCallback &operator=(const IXAudio2EngineCallback &);
};

struct IXAudio2VoiceCallback { /* Size=0x4 */
    virtual void OnVoiceProcessingPassStart(UINT32 BytesRequired);
    virtual void OnVoiceProcessingPassEnd();
    virtual void OnStreamEnd();
    virtual void OnBufferStart(void *pBufferContext);
    virtual void OnBufferEnd(void *pBufferContext);
    virtual void OnLoopEnd(void *pBufferContext);
    virtual void OnVoiceError(void *pBufferContext, HRESULT Error);
    IXAudio2VoiceCallback(const IXAudio2VoiceCallback &);
    IXAudio2VoiceCallback();
    IXAudio2VoiceCallback &operator=(const IXAudio2VoiceCallback &);
};

enum XAUDIO2_XBOX_HWTHREAD_SPECIFIER {
    XboxThread0 = 0x0001,
    XboxThread1 = 0x0002,
    XboxThread2 = 0x0004,
    XboxThread3 = 0x0008,
    XboxThread4 = 0x0010,
    XboxThread5 = 0x0020,
    XAUDIO2_ANY_PROCESSOR = 0x0010,
    XAUDIO2_DEFAULT_PROCESSOR = 0x0010,
};

struct WAVEFORMATEXTENSIBLE { /* Size=0x28 */
    /* 0x0000 */ tWAVEFORMATEX Format;
    /* 0x0012 */ union {
        WORD wValidBitsPerSample;
        WORD wSamplesPerBlock;
        WORD wReserved;
    } Samples;
    /* 0x0014 */ DWORD dwChannelMask;
    /* 0x0018 */ _GUID SubFormat;
};

enum XAUDIO2_DEVICE_ROLE {
    NotDefaultDevice = 0x0000,
    DefaultConsoleDevice = 0x0001,
    DefaultMultimediaDevice = 0x0002,
    DefaultCommunicationsDevice = 0x0004,
    DefaultGameDevice = 0x0008,
    GlobalDefaultDevice = 0x000f,
    InvalidDeviceRole = 0xf0,
};

struct XAUDIO2_DEVICE_DETAILS { /* Size=0x42c */
    /* 0x0000 */ WCHAR DeviceID[256];
    /* 0x0200 */ WCHAR DisplayName[256];
    /* 0x0400 */ XAUDIO2_DEVICE_ROLE Role;
    /* 0x0404 */ WAVEFORMATEXTENSIBLE OutputFormat;
};

struct XAUDIO2_DEBUG_CONFIGURATION { /* Size=0x18 */
    /* 0x0000 */ UINT32 TraceMask;
    /* 0x0004 */ UINT32 BreakMask;
    /* 0x0008 */ BOOL LogThreadID;
    /* 0x000c */ BOOL LogFileline;
    /* 0x0010 */ BOOL LogFunctionName;
    /* 0x0014 */ BOOL LogTiming;
};

struct XAUDIO2_PERFORMANCE_DATA { /* Size=0x40 */
    /* 0x0000 */ UINT64 AudioCyclesSinceLastQuery;
    /* 0x0008 */ UINT64 TotalCyclesSinceLastQuery;
    /* 0x0010 */ UINT32 MinimumCyclesPerQuantum;
    /* 0x0014 */ UINT32 MaximumCyclesPerQuantum;
    /* 0x0018 */ UINT32 MemoryUsageInBytes;
    /* 0x001c */ UINT32 CurrentLatencyInSamples;
    /* 0x0020 */ UINT32 GlitchesSinceEngineStarted;
    /* 0x0024 */ UINT32 ActiveSourceVoiceCount;
    /* 0x0028 */ UINT32 TotalSourceVoiceCount;
    /* 0x002c */ UINT32 ActiveSubmixVoiceCount;
    /* 0x0030 */ UINT32 ActiveResamplerCount;
    /* 0x0034 */ UINT32 ActiveMatrixMixCount;
    /* 0x0038 */ UINT32 ActiveXmaSourceVoices;
    /* 0x003c */ UINT32 ActiveXmaStreams;
};

struct IXAudio2 : public IUnknown { /* Size=0x4 */
    /* 0x0000: fields for IUnknown */
    virtual HRESULT QueryInterface(const _GUID &riid, void **ppvObject) = 0;
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    virtual HRESULT GetDeviceCount(UINT32 *);
    virtual HRESULT GetDeviceDetails(UINT32, XAUDIO2_DEVICE_DETAILS *);
    virtual HRESULT Initialize(UINT32, XAUDIO2_XBOX_HWTHREAD_SPECIFIER);
    virtual HRESULT RegisterForCallbacks(IXAudio2EngineCallback *pCallback);
    virtual void UnregisterForCallbacks(IXAudio2EngineCallback *pCallback);
    virtual HRESULT CreateSourceVoice(
        IXAudio2SourceVoice **ppSourceVoice,
        const tWAVEFORMATEX *pSourceFormat,
        UINT32 Flags,
        float MaxFrequencyRatio,
        IXAudio2VoiceCallback *pCallback,
        const XAUDIO2_VOICE_SENDS *pSendList,
        const XAUDIO2_EFFECT_CHAIN *pEffectChain
    );
    virtual HRESULT CreateSubmixVoice(
        IXAudio2SubmixVoice **ppSubmixVoice,
        UINT32 InputChannels,
        UINT32 InputSampleRate,
        UINT32 Flags,
        UINT32 ProcessingStage,
        const XAUDIO2_VOICE_SENDS *pSendList,
        const XAUDIO2_EFFECT_CHAIN *pEffectChain
    );
    virtual HRESULT CreateMasteringVoice(
        IXAudio2MasteringVoice **ppMasteringVoice,
        UINT32 InputChannels,
        UINT32 InputSampleRate,
        UINT32 Flags,
        UINT32 DeviceIndex,
        const XAUDIO2_EFFECT_CHAIN *pEffectChain
    );
    virtual HRESULT StartEngine();
    virtual void StopEngine();
    virtual HRESULT CommitChanges(UINT32 OperationSet);
    virtual void GetPerformanceData(XAUDIO2_PERFORMANCE_DATA *pPerfData);
    virtual void SetDebugConfiguration(
        const XAUDIO2_DEBUG_CONFIGURATION *pDebugConfiguration, void *pReserved
    );
    IXAudio2(const IXAudio2 &);
    IXAudio2();
    IXAudio2 &operator=(const IXAudio2 &);
};

struct IXAudioRefCount { /* Size=0x4 */
    virtual ULONG AddRef();
    virtual ULONG Release();
    IXAudioRefCount(const IXAudioRefCount &);
    IXAudioRefCount();
    IXAudioRefCount &operator=(const IXAudioRefCount &);
};

struct IXAudioBatchAllocator : public IXAudioRefCount { /* Size=0x4 */
    /* 0x0000: fields for IXAudioRefCount */
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    virtual void GrowHeap(UINT32);
    virtual DWORD CreateHeap(UINT32);
    virtual UINT32 GetFreeHeapSize();
    virtual void *Alloc(UINT32);
    IXAudioBatchAllocator(const IXAudioBatchAllocator &);
    IXAudioBatchAllocator();
    IXAudioBatchAllocator &operator=(const IXAudioBatchAllocator &);
};

// Undo the #pragma pack(push, 1) directive at the top of this file
#pragma pack(pop)
