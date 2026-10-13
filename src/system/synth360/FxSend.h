#pragma once
#include "Synth.h"
#include "synth/FxSend.h"
#include "synth360/Voice.h"
#include "xdk/XAUDIO2.h"

class FxSend360 {
public:
    virtual ~FxSend360();
    virtual void SyncEffectParams(IXAudio2SubmixVoice *) const = 0;
    virtual bool IsStandard() const { return true; }
    virtual void AddOwnerVoice(Voice *);
    virtual void RemoveOwnerVoice(Voice *);
    virtual IUnknown *CreateFx() = 0;

    FxSend360(FxSend *);
    void SyncEffectParams();
    void UpdateVolumes();
    void Cleanup();
    void CleanChain();
    void Refresh(std::vector<FxSend *> &);

    bool HasVoices() const { return !mVoices.empty(); }
    IXAudio2Voice *InputVoice() { return mInputVoice; }

protected:
    virtual void InitParams(IXAudio2SubmixVoice *, int) {}

private:
    IXAudio2Voice *OutputVoice();
    void UpdateVoiceMatrices();
    void CreateInputVoice();
    void Reconnect();
    void CreateVoice(int, int);

    IXAudio2SubmixVoice *mInputVoice; // 0x4
    std::vector<IXAudio2SubmixVoice *> mVoices; // 0x8
    std::vector<int> mVoicesChannels; // 0x14
    std::vector<IUnknown *> mFx; // 0x20
    FxSend *mThis; // 0x2c
    bool mParamsDirty; // 0x30
    std::vector<Voice *> mOwnerVoices; // 0x34
};
