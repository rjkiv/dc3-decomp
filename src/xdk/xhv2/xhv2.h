#pragma once
#include "xdk/win_types.h"
#include "xdk/XAUDIO2.h"
#include "xdk/XAPILIB.h"

#ifdef __cplusplus
extern "C" {
#endif

struct IXHV2Engine { /* Size=0x4 */
    virtual ULONG AddRef();
    virtual ULONG Release();
    virtual HRESULT Lock(UINT32 LockType);
    virtual HRESULT StartLocalProcessingModes(UINT32 Port, void **Modes, UINT32 NumModes);
    virtual HRESULT StopLocalProcessingModes(UINT32 Port, void **Modes, UINT32 NumModes);
    virtual HRESULT StartRemoteProcessingModes(XUID Xuid, void **Modes, UINT32 NumModes);
    virtual HRESULT StopRemoteProcessingModes(XUID Xuid, void **Modes, UINT32 NumModes);
    virtual HRESULT SetMaxDecodePackets(UINT32 MaxDecodePackets);
    virtual HRESULT RegisterLocalTalker(UINT32 Port);
    virtual HRESULT UnregisterLocalTalker(UINT32 Port);
    virtual HRESULT RegisterRemoteTalker(
        XUID Xuid,
        XAUDIO2_EFFECT_CHAIN *TalkerFX,
        XAUDIO2_EFFECT_CHAIN *PairFX,
        struct IXAudio2SubmixVoice *OutputVoice
    );
    virtual HRESULT UnregisterRemoteTalker(XUID Xuid);
    virtual HRESULT GetRemoteTalkers(UINT32 *RemoteTalkerCount, UINT64 *RemoteTalkers);
    virtual BOOL IsHeadsetPresent(UINT32 Port);
    virtual BOOL IsLocalTalking(UINT32 Port);
    virtual BOOL IsRemoteTalking(XUID Xuid);
    virtual HRESULT
    SetRemoteTalkerOutputVoice(XUID Xuid, IXAudio2SubmixVoice *OutputVoice);
    virtual HRESULT SetRemoteTalkerEffectParam(
        XUID Xuid,
        INT PairEffect,
        UINT32 uEffectIndex,
        const void *pParameters,
        UINT32 uParametersByteSize
    );
    virtual DWORD GetDataReadyFlags();
    virtual HRESULT
    GetLocalChatData(UINT32 Port, BYTE *Buffer, UINT32 *Size, UINT32 *Packets);
    virtual HRESULT NuiGetLocalChatData(
        BYTE *Buffer,
        UINT32 *Size,
        UINT32 *Packets,
        NUI_TALKER_POSITION *Positions,
        UINT32 *NumPositons
    );
    virtual HRESULT SetPlaybackPriority(XUID Xuid, UINT32 Port, UINT32 Priority);
    virtual HRESULT SubmitIncomingChatData(XUID Xuid, const BYTE *Data, UINT32 *Size);
    virtual BOOL IsSharedMicPresent(UINT32 Port);
    virtual HRESULT GetVoiceVolume(UINT32 Port, float *pfVoiceVolume);
    virtual HRESULT GetGameVolume(float *pfGameVolume);
    virtual BOOL IsVoiceOverSpeakers(UINT32 Port);
    virtual void EnableDucking(BOOL fEnable);
    virtual BOOL IsDuckingEnabled();

    IXHV2Engine(const IXHV2Engine &);
    IXHV2Engine();
    IXHV2Engine &operator=(const IXHV2Engine &);
};

struct XHV_INIT_PARAMS { /* Size=0x34 */
    /* 0x0000 */ DWORD dwMaxRemoteTalkers;
    /* 0x0004 */ DWORD dwMaxLocalTalkers;
    /* 0x0008 */ void **localTalkerEnabledModes;
    /* 0x000c */ DWORD dwNumLocalTalkerEnabledModes;
    /* 0x0010 */ void **remoteTalkerEnabledModes;
    /* 0x0014 */ DWORD dwNumRemoteTalkerEnabledModes;
    /* 0x0018 */ BOOL bCustomVADProvided;
    /* 0x001c */ BOOL bRelaxPrivileges;
    /* 0x0020 */ void (*pfnMicrophoneRawDataReady)(DWORD, void *, DWORD, INT *);
    /* 0x0024 */ XAUDIO2_EFFECT_CHAIN **ppfxDefaultRemoteTalkerFX;
    /* 0x0028 */ XAUDIO2_EFFECT_CHAIN **ppfxDefaultTalkerPairFX;
    /* 0x002c */ XAUDIO2_EFFECT_CHAIN *pfxOutputFX;
    /* 0x0030 */ IXAudio2 *pXAudio2;
};

HRESULT
XHV2CreateEngine(XHV_INIT_PARAMS *Params, HANDLE *WorkerThread, IXHV2Engine **Engine);

extern void *_xhv_voicechat_mode;
extern void *_xhv_loopback_mode;

#ifdef __cplusplus
}
#endif
