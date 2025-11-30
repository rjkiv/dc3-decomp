#pragma once
#include "Synth.h"
#include "stl/_vector.h"
#include "synth/FxSend.h"
#include "synth_xbox/Voice.h"
#include "xdk/xapilibi/xbase.h"
#include "xdk/xaudio2/xaudio2.h"

class FxSend360 {
public:
    virtual ~FxSend360();
    virtual void AddOwnerVoice(Voice *);
    virtual void RemoveOwnerVoice(Voice *);

    FxSend360(FxSend *);
    void SyncEffectParams();
    void UpdateVolumes();
    void Cleanup();
    void CleanChain();
    void Refresh(std::vector<FxSend *> &);

    int unk4;
    std::vector<IXAudio2SubmixVoice *> unk8;
    std::vector<int> unk14;
    std::vector<IUnknown *> unk20;
    FxSend *mThis; // 0x2c
    bool unk30;
    std::vector<Voice *> unk34;

private:
    struct IXAudio2Voice *OutputVoice();
    void UpdateVoiceMatrices();
    void CreateInputVoice();
};
