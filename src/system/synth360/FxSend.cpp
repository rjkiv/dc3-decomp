#include "FxSend.h"
#include "Synth.h"
#include "Voice.h"
#include "math/Decibels.h"
#include "math/Utl.h"
#include "os/Debug.h"
#include "os/Timer.h"
#include "synth/FxSend.h"
#include "synth/Synth.h"
#include "synth360/Synth.h"
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

void FxSend360::UpdateVoiceMatrices() {
    float gain = DbToRatio(mThis->InputGain());
    if (mThis->Bypass()) {
        gain = 0.0f;
    }
    float reverbMix = DbToRatio(mThis->ReverbMixDb());
    SendChannels channel = mThis->GetChannels();
    gain = Max(gain, 1E-10f);
    reverbMix = Max(reverbMix, 1E-10f);
    switch (channel) {
    case kSendAll: {
        const float floats[12] = { gain, 0, 0, 0, 0, 0, 0, gain, 0, 0, 0, 0 };
        mOutputVoice->SetOutputMatrix(mVoices[0], 6, 2, floats, 0);
        const float floats2[6] = { 0, 0, gain, 0, 0, 0 };
        mOutputVoice->SetOutputMatrix(mVoices[1], 6, 1, floats2, 0);
        const float floats3[12] = { 0, 0, 0, 0, gain, 0, 0, 0, 0, 0, 0, gain };
        mOutputVoice->SetOutputMatrix(mVoices[2], 6, 2, floats3, 0);
        gain = 1.0f;
        const float floats4[12] = { gain, 0, 0, gain, 0, 0, 0, 0, 0, 0, 0, 0 };
        if (mVoices.front()) {
            mVoices.front()->SetOutputMatrix(OutputVoice(), 2, 6, floats4, 0);
        }
        const float floats5[6] = { 0, 0, 1.0f, 0, 0, 0 };
        if (mVoices[1]) {
            mVoices[1]->SetOutputMatrix(OutputVoice(), 1, 6, floats5, 0);
        }
        const float floats6[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 1.0f, 0, 0, 1.0f };
        if (mVoices[2]) {
            mVoices[2]->SetOutputMatrix(OutputVoice(), 2, 6, floats6, 0);
        }

        if (mThis->ReverbEnabled()) {
            const float floats7[12] = {
                reverbMix, 0, 0, reverbMix, 0, 0, 0, 0, 0, 0, 0, 0
            };
            mVoices[0]->SetOutputMatrix(TheXboxSynth->ReverbSendVoice(), 2, 6, floats7, 0);
            const float floats8[6] = { 0, 0, reverbMix, 0, 0, 0 };
            mVoices[1]->SetOutputMatrix(TheXboxSynth->ReverbSendVoice(), 1, 6, floats8, 0);
            const float floats9[12] = {
                0, 0, 0, 0, 0, 0, 0, 0, reverbMix, 0, 0, reverbMix
            };
            mVoices[2]->SetOutputMatrix(TheXboxSynth->ReverbSendVoice(), 2, 6, floats9, 0);
        }

    } break;
    case kSendCenter: {
        const float floats[6] = { gain / 2, gain / 2, gain, 0, gain / 4, gain / 4 };
        mOutputVoice->SetOutputMatrix(mVoices[0], 6, 1, floats, 0);
        if (mThis->ReverbEnabled()) {
            const float floats2[6] = { 0, 0, reverbMix, 0, 0, 0 };
            mVoices[0]->SetOutputMatrix(TheXboxSynth->ReverbSendVoice(), 1, 6, floats2, 0);
        }

    } break;
    case kSendStereo: {
        const float floats[12] = { gain, 0,    gain * 0.7f, 0, gain * 0.3f, 0,
                                   0,    gain, gain * 0.7f, 0, 0,           gain * 0.3f };
        mOutputVoice->SetOutputMatrix(mVoices[0], 6, 2, floats, 0);
        if (mThis->ReverbEnabled()) {
            const float floats2[12] = {
                reverbMix, 0, 0, reverbMix, 0, 0, 0, 0, 0, 0, 0, 0
            };
            mVoices[0]->SetOutputMatrix(TheXboxSynth->ReverbSendVoice(), 2, 6, floats2, 0);
        }

    }

    break;
    case kSendAllXMix: {
        const float floats[12] = {
            gain, 0, gain / 4, 0, 0, 0, 0, gain, gain / 4, 0, 0, 0
        };
        mOutputVoice->SetOutputMatrix(mVoices[0], 6, 2, floats, 0);
        const float floats2[6] = { 0, 0, gain / 10, 0, 0, 0 };
        mOutputVoice->SetOutputMatrix(mVoices[1], 6, 1, floats2, 0);
        const float floats3[12] = {
            0, 0, gain / 4, 0, gain, 0, 0, 0, gain / 4, 0, 0, gain
        };
        mOutputVoice->SetOutputMatrix(mVoices[2], 6, 2, floats3, 0);
        gain = 1.0f;
        const float floats4[12] = { gain, 0, 0, gain, 0, 0, 0, 0, 0, 0, 0, 0 };
        if (mVoices.front()) {
            mVoices.front()->SetOutputMatrix(OutputVoice(), 2, 6, floats4, 0);
        }
        const float floats5[6] = { 0, 0, gain, 0, 0, 0 };
        if (mVoices[1]) {
            mVoices[1]->SetOutputMatrix(OutputVoice(), 1, 6, floats5, 0);
        }
        const float floats6[12] = { 0, 0, 0, 0, 0, 0, 0, 0, gain, 0, 0, gain };
        if (mVoices[2]) {
            mVoices[2]->SetOutputMatrix(OutputVoice(), 2, 6, floats6, 0);
        }

        if (mThis->ReverbEnabled()) {
            const float floats7[12] = {
                reverbMix, 0, 0, reverbMix, 0, 0, 0, 0, 0, 0, 0, 0
            };
            mVoices[0]->SetOutputMatrix(TheXboxSynth->ReverbSendVoice(), 2, 6, floats7, 0);
            const float floats8[6] = { 0, 0, reverbMix, 0, 0, 0 };
            mVoices[1]->SetOutputMatrix(TheXboxSynth->ReverbSendVoice(), 1, 6, floats8, 0);
            const float floats9[12] = {
                0, 0, 0, 0, 0, 0, 0, 0, reverbMix, 0, 0, reverbMix
            };
            mVoices[2]->SetOutputMatrix(TheXboxSynth->ReverbSendVoice(), 2, 6, floats9, 0);
        }

    } break;
    default:
        MILO_ASSERT(0, 0x136);
        break;
    }
}

void FxSend360::UpdateVolumes() {
    if (mOutputVoice) {
        float wetGain = DbToRatio(mThis->WetGain());
        float dryGain = DbToRatio(mThis->DryGain());
        if (mThis->Bypass()) {
            dryGain = 1.0f;
            wetGain = 0;
        }

        dryGain = Max(dryGain, 1E-10F);
        wetGain = Max(wetGain, 1E-10F);

        const float floats[36] = { dryGain, 0, 0, 0, 0, 0, 0, dryGain, 0, 0, 0, 0, 0, 0,
                                   dryGain, 0, 0, 0, 0, 0, 0, dryGain, 0, 0, 0, 0, 0, 0,
                                   dryGain, 0, 0, 0, 0, 0, 0, dryGain };
        HRESULT hr = mOutputVoice->SetOutputMatrix(OutputVoice(), 6, 6, floats, 0);
        MILO_ASSERT(SUCCEEDED(hr), 0x203);
        UpdateVoiceMatrices();
        for (int i = 0; i < mVoices.size(); i++) {
            hr = mVoices[i]->SetVolume(wetGain, 0);
            MILO_ASSERT(SUCCEEDED(hr), 0x212);
        }

        if (IsStandard()) {
            for (int i = 0; i != mVoices.size(); i++) {
                SyncEffectParams(mVoices[i]);
            }
        }
    }
}
