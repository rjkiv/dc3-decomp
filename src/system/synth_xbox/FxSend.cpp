#include "FxSend.h"
#include "Synth.h"
#include "os/Debug.h"
#include "os/Timer.h"
#include "synth/FxSend.h"
#include "synth/Synth.h"

FxSend360::FxSend360(FxSend *fx) : mOutputVoice(0), mThis(fx), unk30(true) {
    TheXboxSynth->AddFxSend(this);
    MILO_ASSERT(mThis, 0x19);
}

FxSend360::~FxSend360() {
    if (TheXboxSynth)
        TheXboxSynth->RemoveFxSend(this);
    CleanChain();
}

void FxSend360::AddOwnerVoice(Voice *v) { mOwnerVoices.push_back(v); }

void FxSend360::RemoveOwnerVoice(Voice *v) {
    auto itFind = mOwnerVoices.end();
    FOREACH (it, mOwnerVoices) {
        if (*it == v) {
            itFind = it;
        }
    }
    MILO_ASSERT(itFind != mOwnerVoices.end(), 0x265);
    mOwnerVoices.erase(itFind);
}

void FxSend360::SyncEffectParams() {
    START_AUTO_TIMER("voice_cs");
    if (!mThis->UpdatesEnabled()) {
        unk30 = true;
        return;
    } else {
        if (unk30 || mThis->UpdatesEnabled()) {
            for (int i = 0; i != mVoices.size(); i++) {
                SyncEffectParams(mVoices[i]);
            }
        }
        unk30 = false;
    }
}

void FxSend360::Refresh(std::vector<FxSend *> &sends) {
    if (TheXboxSynth) {
        for (int i = sends.size() - 1; i >= 0; i--) {
            FxSend360 *send360 = dynamic_cast<FxSend360 *>(sends[i]);
            send360->Cleanup();
        }
        for (int i = 0; i < sends.size(); i++) {
            FxSend360 *send360 = dynamic_cast<FxSend360 *>(sends[i]);
            send360->Reconnect();
        }
    }
}

void FxSend360::Cleanup() {
    std::vector<Voice *> voices(mOwnerVoices);
    for (int i = 0; i < voices.size(); i++) {
        voices[i]->SetSend(nullptr);
    }
    if (mOutputVoice) {
        mOutputVoice->DestroyVoice();
        mOutputVoice = nullptr;
    }
    MILO_ASSERT(mVoices.size() == mFx.size(), 0x2A);
    for (int i = 0; i != mVoices.size(); i++) {
        mVoices[i]->DestroyVoice();
        if (mFx[i]) {
            mFx[i]->Release();
            mFx[i] = nullptr;
        }
    }
    mVoices.clear();
    mFx.clear();
}

void FxSend360::CleanChain() {
    std::vector<FxSend *> sends;
    mThis->BuildChainVector(sends);
    for (int i = sends.size() - 1; i >= 0; i--) {
        FxSend360 *send360 = dynamic_cast<FxSend360 *>(sends[i]);
        send360->Cleanup();
    }
}

IXAudio2Voice *FxSend360::OutputVoice() {
    if (mThis->NextSend()) {
        FxSend360 *send = dynamic_cast<FxSend360 *>(mThis->NextSend());
        MILO_ASSERT(send, 0x225);
        return send->mOutputVoice;
    } else {
        Synth360 *synth = dynamic_cast<Synth360 *>(TheSynth);
        return synth->OutputVoice();
    }
}

void FxSend360::Reconnect() {
    if (OutputVoice()) {
        switch (mThis->GetChannels()) {
        case kSendAll:
        case kSendAllXMix:
            CreateVoice(0, 1);
            CreateVoice(2, -1);
            CreateVoice(4, 5);
            break;
        case kSendCenter:
            CreateVoice(2, -1);
            break;
        case kSendStereo:
            CreateVoice(0, 1);
            break;
        default:
            MILO_ASSERT(0, 0x150);
            break;
        }
        CreateInputVoice();
        SyncEffectParams();
        UpdateVolumes();
    }
}
