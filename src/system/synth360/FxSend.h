#pragma once
#include "Synth.h"
#include "synth/FxSend.h"
#include "synth360/Voice.h"
#include "xdk/XAUDIO2.h"

class FxSend360 {
public:
    virtual ~FxSend360();
    virtual void SyncEffectParams(IXAudio2SubmixVoice *voice) const = 0;
    virtual bool IsStandard() const { return true; }
    virtual void AddOwnerVoice(Voice *pVoice);
    virtual void RemoveOwnerVoice(Voice *pVoice);

    FxSend360(FxSend *me);

    bool Initialized() { return !mVoices.empty(); }
    void Refresh(std::vector<FxSend *> &chain);
    IXAudio2Voice *InputVoice() { return mInputVoice; }
    void SyncEffectParams();
    void UpdateVolumes();
    void Cleanup();
    void CleanChain();

protected:
    virtual IUnknown *CreateFx() = 0;
    virtual void InitParams(IXAudio2SubmixVoice *voice, int numChannels) {}

private:
    IXAudio2Voice *OutputVoice();
    void UpdateVoiceMatrices();
    void CreateInputVoice();
    void Reconnect();
    void CreateVoice(int channel1, int channel2);

    IXAudio2SubmixVoice *mInputVoice; // 0x4
    std::vector<IXAudio2SubmixVoice *> mVoices; // 0x8
    std::vector<int> mVoicesChannels; // 0x14
    std::vector<IUnknown *> mFx; // 0x20
    FxSend *mThis; // 0x2c
    bool mParamsDirty; // 0x30
    std::vector<Voice *> mOwnerVoices; // 0x34
};
