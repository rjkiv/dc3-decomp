#include "FxSend.h"
#include "Synth.h"
#include "os/Debug.h"
#include "os/Timer.h"
#include "synth/FxSend.h"
#include "synth/Synth.h"
#include "xdk/win_types.h"
#include "xdk/xaudio2/xaudio2.h"

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

void FxSend360::CreateInputVoice() {
    MILO_ASSERT(OutputVoice(), 0x177);
    unsigned int numVoices = mVoices.size();
// clang-format off
    XAUDIO2_SEND_DESCRIPTOR allDescs[4] = {
        { 0, mVoices[0] },
        { 0, numVoices >= 2 ? mVoices[1] : nullptr },
        { 0, numVoices >= 3 ? mVoices[2] : nullptr },
        { 0, OutputVoice() }
    };

    XAUDIO2_SEND_DESCRIPTOR stereoDescs[2] = {
        { 0, mVoices[0] }, { 0, OutputVoice() }
    };

    XAUDIO2_SEND_DESCRIPTOR centerDescs[2] = {
        { 0, mVoices[0] }, { 0, OutputVoice() }
    };

    HRESULT hr = S_OK;
    int stage = mThis->Stage() << 1;
    switch (mThis->GetChannels()) {
        default: 
            MILO_FAIL("FxSend: Unknown Channels");  
        case kSendAll: 
        case kSendAllXMix:{
            XAUDIO2_VOICE_SENDS sends;
            sends.SendCount = 4;
            sends.pSends = allDescs;
            hr = TheXboxSynth->GetXAudio()->CreateSubmixVoice(&mOutputVoice, 6, 48000, 0, stage, &sends, nullptr);
            break;
        }
        case kSendCenter: {
            XAUDIO2_VOICE_SENDS sends;
            sends.SendCount = 2;
            sends.pSends = centerDescs;
            hr = TheXboxSynth->GetXAudio()->CreateSubmixVoice(&mOutputVoice, 6, 48000, 0, stage, &sends, nullptr);
            break;
        }
        case kSendStereo: {
            XAUDIO2_VOICE_SENDS sends;
            sends.SendCount = 2;
            sends.pSends = stereoDescs;
            hr = TheXboxSynth->GetXAudio()->CreateSubmixVoice(&mOutputVoice, 6, 48000, 0, stage, &sends, nullptr);
            break;
        }
    }
    MILO_ASSERT(SUCCEEDED(hr), 0x1A6);
// clang-format on
}

void FxSend360::CreateVoice(int i1, int i2) {
    int stage = mThis->Stage() << 1;
// clang-format off
    XAUDIO2_SEND_DESCRIPTOR desc = { 0, OutputVoice() };
    std::vector<XAUDIO2_SEND_DESCRIPTOR> descs;
    if(desc.pOutputVoice){
        descs.push_back(desc);
    }
    if(mThis->ReverbEnabled()){
        XAUDIO2_SEND_DESCRIPTOR desc2 = { 0, TheXboxSynth->ReverbSendVoice() };
        descs.push_back(desc2);
    }
    mFx.push_back(CreateFx());
    MILO_ASSERT(mFx.back(), 0x1CE);

// clang-format on
}
