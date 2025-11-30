#pragma once

#include "xdk/win_types.h"
#include "xdk/xapilibi/xbase.h"

struct IXAudio2Voice { /* Size=0x4 */

    virtual void GetVoiceDetails(XAUDIO2_VOICE_DETAILS *);
    virtual HRESULT SetOutputVoices(const XAUDIO2_VOICE_SENDS *);
    virtual HRESULT SetEffectChain(const XAUDIO2_EFFECT_CHAIN *);
    virtual HRESULT EnableEffect(UINT32, UINT32);
    virtual HRESULT DisableEffect(UINT32, UINT32);
    virtual void GetEffectState(UINT32, BOOL *);
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
    virtual void GetEffectState(UINT32, UINT32 *) = 0;
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
