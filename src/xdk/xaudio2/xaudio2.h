#pragma once
#include "xdk/win_types.h"
#include "xdk/unknwn.h"
#include "xdk/xapilibi/xbase.h"
#include "xapobase.h"

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

struct IXAudio2Voice { /* Size=0x4 */

    virtual void GetVoiceDetails(XAUDIO2_VOICE_DETAILS *);
    virtual HRESULT SetOutputVoices(const XAUDIO2_VOICE_SENDS *);
    virtual HRESULT SetEffectChain(const XAUDIO2_EFFECT_CHAIN *);
    virtual HRESULT EnableEffect(UINT32, UINT32);
    virtual HRESULT DisableEffect(UINT32, UINT32);
    virtual void GetEffectState(INT, BOOL *);
    virtual HRESULT SetEffectParameters(UINT32, const void *, UINT32, UINT32);
    virtual HRESULT GetEffectParameters(UINT32, void *, UINT32);
    virtual HRESULT SetFilterParameters(const XAUDIO2_FILTER_PARAMETERS *, UINT32);
    virtual void GetFilterParameters(XAUDIO2_FILTER_PARAMETERS *);
    virtual HRESULT
    SetOutputFilterParameters(IXAudio2Voice *, const XAUDIO2_FILTER_PARAMETERS *, UINT32);
    virtual void GetOutputFilterParameters(IXAudio2Voice *, XAUDIO2_FILTER_PARAMETERS *);
    virtual HRESULT SetVolume(float, UINT32);
    virtual void GetVolume(float *);
    virtual HRESULT SetChannelVolumes(UINT32, const float *, UINT32);
    virtual void GetChannelVolumes(UINT32, float *);
    virtual HRESULT
    SetOutputMatrix(IXAudio2Voice *, UINT32, UINT32, const float *, UINT32);
    virtual void GetOutputMatrix(IXAudio2Voice *, UINT32, UINT32, float *);
    virtual void DestroyVoice();
    IXAudio2Voice(const IXAudio2Voice &);
    IXAudio2Voice();
    IXAudio2Voice &operator=(const IXAudio2Voice &);
};

struct IXAudio2SubmixVoice : public IXAudio2Voice { /* Size=0x4 */
    /* 0x0000: fields for IXAudio2Voice */

    virtual void GetVoiceDetails(XAUDIO2_VOICE_DETAILS *) = 0;
    virtual HRESULT SetOutputVoices(const XAUDIO2_VOICE_SENDS *) = 0;
    virtual HRESULT SetEffectChain(const XAUDIO2_EFFECT_CHAIN *) = 0;
    virtual HRESULT EnableEffect(UINT32, UINT32) = 0;
    virtual HRESULT DisableEffect(UINT32, UINT32) = 0;
    virtual void GetEffectState(INT, BOOL *) = 0;
    virtual HRESULT SetEffectParameters(UINT32, const void *, UINT32, UINT32) = 0;
    virtual HRESULT GetEffectParameters(UINT32, void *, UINT32) = 0;
    virtual HRESULT SetFilterParameters(const XAUDIO2_FILTER_PARAMETERS *, UINT32) = 0;
    virtual void GetFilterParameters(XAUDIO2_FILTER_PARAMETERS *) = 0;
    virtual HRESULT SetOutputFilterParameters(
        IXAudio2Voice *, const XAUDIO2_FILTER_PARAMETERS *, UINT32
    ) = 0;
    virtual void
    GetOutputFilterParameters(IXAudio2Voice *, XAUDIO2_FILTER_PARAMETERS *) = 0;
    virtual HRESULT SetVolume(float, UINT32) = 0;
    virtual void GetVolume(float *) = 0;
    virtual HRESULT SetChannelVolumes(UINT32, const float *, UINT32) = 0;
    virtual void GetChannelVolumes(UINT32, float *) = 0;
    virtual HRESULT
    SetOutputMatrix(IXAudio2Voice *, UINT32, UINT32, const float *, UINT32) = 0;
    virtual void GetOutputMatrix(IXAudio2Voice *, UINT32, UINT32, float *) = 0;
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
public:
    virtual void GetVoiceDetails(XAUDIO2_VOICE_DETAILS *) = 0;
    virtual HRESULT SetOutputVoices(const XAUDIO2_VOICE_SENDS *) = 0;
    virtual HRESULT SetEffectChain(const XAUDIO2_EFFECT_CHAIN *) = 0;
    virtual HRESULT EnableEffect(UINT32, UINT32) = 0;
    virtual HRESULT DisableEffect(UINT32, UINT32) = 0;
    virtual void GetEffectState(INT, BOOL *) = 0;
    virtual HRESULT SetEffectParameters(UINT32, const void *, UINT32, UINT32) = 0;
    virtual HRESULT GetEffectParameters(UINT32, void *, UINT32) = 0;
    virtual HRESULT SetFilterParameters(const XAUDIO2_FILTER_PARAMETERS *, UINT32) = 0;
    virtual void GetFilterParameters(XAUDIO2_FILTER_PARAMETERS *) = 0;
    virtual HRESULT SetOutputFilterParameters(
        IXAudio2Voice *, const XAUDIO2_FILTER_PARAMETERS *, UINT32
    ) = 0;
    virtual void
    GetOutputFilterParameters(IXAudio2Voice *, XAUDIO2_FILTER_PARAMETERS *) = 0;
    virtual HRESULT SetVolume(float, UINT32) = 0;
    virtual void GetVolume(float *) = 0;
    virtual HRESULT SetChannelVolumes(UINT32, const float *, UINT32) = 0;
    virtual void GetChannelVolumes(UINT32, float *) = 0;
    virtual HRESULT
    SetOutputMatrix(IXAudio2Voice *, UINT32, UINT32, const float *, UINT32) = 0;
    virtual void GetOutputMatrix(IXAudio2Voice *, UINT32, UINT32, float *) = 0;
    virtual void DestroyVoice() = 0;
    virtual HRESULT Start(UINT32, UINT32);
    virtual HRESULT Stop(UINT32, UINT32);
    virtual HRESULT
    SubmitSourceBuffer(const XAUDIO2_BUFFER *, const XAUDIO2_BUFFER_WMA *);
    virtual HRESULT FlushSourceBuffers();
    virtual HRESULT Discontinuity();
    virtual HRESULT ExitLoop(UINT32);
    virtual void GetState(XAUDIO2_VOICE_STATE *);
    virtual HRESULT SetFrequencyRatio(float, UINT32);
    virtual void GetFrequencyRatio(float *);
    virtual HRESULT SetSourceSampleRate(UINT32);
    IXAudio2SourceVoice(const IXAudio2SourceVoice &);
    IXAudio2SourceVoice();
    IXAudio2SourceVoice &operator=(const IXAudio2SourceVoice &);
};

struct IXAudio2MasteringVoice : public IXAudio2Voice { /* Size=0x4 */
    /* 0x0000: fields for IXAudio2Voice */
public:
    virtual void GetVoiceDetails(XAUDIO2_VOICE_DETAILS *) = 0;
    virtual HRESULT SetOutputVoices(const XAUDIO2_VOICE_SENDS *) = 0;
    virtual HRESULT SetEffectChain(const XAUDIO2_EFFECT_CHAIN *) = 0;
    virtual HRESULT EnableEffect(UINT32, UINT32) = 0;
    virtual HRESULT DisableEffect(UINT32, UINT32) = 0;
    virtual void GetEffectState(INT, BOOL *) = 0;
    virtual HRESULT SetEffectParameters(UINT32, const void *, UINT32, UINT32) = 0;
    virtual HRESULT GetEffectParameters(UINT32, void *, UINT32) = 0;
    virtual HRESULT SetFilterParameters(const XAUDIO2_FILTER_PARAMETERS *, UINT32) = 0;
    virtual void GetFilterParameters(XAUDIO2_FILTER_PARAMETERS *) = 0;
    virtual HRESULT SetOutputFilterParameters(
        IXAudio2Voice *, const XAUDIO2_FILTER_PARAMETERS *, UINT32
    ) = 0;
    virtual void
    GetOutputFilterParameters(IXAudio2Voice *, XAUDIO2_FILTER_PARAMETERS *) = 0;
    virtual HRESULT SetVolume(float, UINT32) = 0;
    virtual void GetVolume(float *) = 0;
    virtual HRESULT SetChannelVolumes(UINT32, const float *, UINT32) = 0;
    virtual void GetChannelVolumes(UINT32, float *) = 0;
    virtual HRESULT
    SetOutputMatrix(IXAudio2Voice *, UINT32, UINT32, const float *, UINT32) = 0;
    virtual void GetOutputMatrix(IXAudio2Voice *, UINT32, UINT32, float *) = 0;
    virtual void DestroyVoice() = 0;
    IXAudio2MasteringVoice(const IXAudio2MasteringVoice &);
    IXAudio2MasteringVoice();
    IXAudio2MasteringVoice &operator=(const IXAudio2MasteringVoice &);
};

struct IXAudioRefCount { /* Size=0x4 */
    virtual UINT32 AddRef();
    virtual UINT32 Release();
    IXAudioRefCount(const IXAudioRefCount &);
    IXAudioRefCount();
    IXAudioRefCount &operator=(const IXAudioRefCount &);
};

struct IXAudioBatchAllocator : public IXAudioRefCount { /* Size=0x4 */
    /* 0x0000: fields for IXAudioRefCount */
    virtual UINT32 AddRef() = 0;
    virtual UINT32 Release() = 0;
    virtual void GrowHeap(UINT32);
    virtual DWORD CreateHeap(UINT32);
    virtual UINT32 GetFreeHeapSize();
    virtual void *Alloc(UINT32);
    IXAudioBatchAllocator(const IXAudioBatchAllocator &);
    IXAudioBatchAllocator();
    IXAudioBatchAllocator &operator=(const IXAudioBatchAllocator &);
};
