#pragma once
#include "xdk/win_types.h"
#include "xdk/XAUDIO2.h"

struct IXHV2Engine { /* Size=0x4 */

    virtual UINT32 AddRef();
    virtual UINT32 Release();
    virtual HRESULT Lock(UINT32);
    virtual HRESULT StartLocalProcessingModes(UINT32, void **, UINT32);
    virtual HRESULT StopLocalProcessingModes(UINT32, void **, UINT32);
    virtual HRESULT StartRemoteProcessingModes(UINT32, void **, UINT32);
    virtual HRESULT StopRemoteProcessingModes(UINT32, void **, UINT32);
    virtual HRESULT SetMaxDecodePackets(UINT32);
    virtual HRESULT RegisterLocalTalker(UINT32);
    virtual HRESULT UnregisterLocalTalker(UINT32);
    virtual HRESULT RegisterRemoteTalker(
        UINT64,
        XAUDIO2_EFFECT_CHAIN *,
        XAUDIO2_EFFECT_CHAIN *,
        struct IXAudio2SubmixVoice *
    );
    virtual HRESULT UnregisterRemoteTalker(UINT64);
    virtual HRESULT GetRemoteTalkers(UINT32 *, UINT64 *);
    virtual BOOL IsHeadsetPresent(UINT32);
    virtual BOOL IsLocalTalking(UINT32);
    virtual BOOL IsRemoteTalking(UINT64);
    virtual HRESULT SetRemoteTalkerOutputVoice(UINT64, IXAudio2SubmixVoice *);
    virtual HRESULT
    SetRemoteTalkerEffectParam(UINT64, DWORD, UINT32, const void *, UINT32);
    virtual DWORD GetDataReadyFlags();
    virtual HRESULT GetLocalChatData(UINT32, unsigned char *, UINT32 *, UINT32 *);
    virtual HRESULT NuiGetLocalChatData(
        unsigned char *, UINT32 *, UINT32 *, NUI_TALKER_POSITION *, UINT32 *
    );
    virtual HRESULT SetPlaybackPriority(UINT64, UINT32, UINT32);
    virtual HRESULT SubmitIncomingChatData(UINT64, const unsigned char *, UINT32 *);
    virtual BOOL IsSharedMicPresent(UINT32);
    virtual HRESULT GetVoiceVolume(UINT32, float *);
    virtual HRESULT GetGameVolume(float *);
    virtual BOOL IsVoiceOverSpeakers(UINT);
    virtual void EnableDucking(INT);
    virtual BOOL IsDuckingEnabled();

    IXHV2Engine(const IXHV2Engine &);
    IXHV2Engine();
    IXHV2Engine &operator=(const IXHV2Engine &);
};
