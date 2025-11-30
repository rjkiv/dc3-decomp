#include "synth_xbox/Mic.h"
#include "macros.h"
#include "math/Decibels.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/Symbol.h"
#include <cstring>

MicManagerXbox *sInstance;

#pragma region MicXbox

MicXbox::MicXbox(int, float volume)
    : unkd(false), unk10(0), mChangeNotify(false), unk18(0), unk301c(0), unk9054(1.0f),
      unk9058(0), unk905c(0), unk9060(0), mVolume(volume), mMute(false), unk906c(0),
      mGain(1.0f), mOutputGain(1.0f), mSensitivity(1.0f), unk907c(0), mDroppedSamples(0),
      unk90c4("generic_usb"), mClipping(false) {
    unk302c->Init(0xc00);
    unk3040->Init(0x6000);
    unk3020.reserve(0x1800);
    memset(unk1c, 0, 0x3000);
}

MicXbox::~MicXbox() {
    if (unkd)
        Stop();
}

bool MicXbox::GetClipping() const { return mClipping; }

float MicXbox::GetGain() const { return mGain; }

int MicXbox::GetDroppedSamples() { return mDroppedSamples; }

float MicXbox::GetOutputGain() const { return mOutputGain; }

float MicXbox::GetSensitivity() const { return mSensitivity; }

void MicXbox::ClearBuffers() {
    unk302c->Reset();
    unk3040->Reset();
}

void MicXbox::SetOutputGain(float f) {
    mOutputGain = f;
    MILO_ASSERT(mOutputGain >= 0.0f, 0x32c);
}

void MicXbox::SetSensitivity(float f) {
    mSensitivity = f;
    MILO_ASSERT(mOutputGain >= 0.0f, 0x337);
}

void MicXbox::SetVolume(float f) { mVolume = DbToRatio(f); }

void MicXbox::SetChangeNotify(bool b) { mChangeNotify = b; }

void MicXbox::SetMute(bool b) { mMute = b; }

bool MicXbox::IsPlaying() { return unk18; }

#pragma endregion MicXbox
#pragma region MicManagerXbox

MicManagerXbox::MicManagerXbox()
    : unk18(-1), unk1c(0), unk2c(0), unk30(false), unk88(-1) {
    for (int i = 4; i != 0; i--) {
        unkc.push_back(0);
    }
    unk20.reserve(4);
    DataRegisterFunc("set_noise_gate", SetNoiseGate);
    DataRegisterFunc("set_low_cut", SetLowCut);
    DataRegisterFunc("set_local_gain", SetLocalGain);
    DataRegisterFunc("set_remote_gain", SetRemoteGain);
    DataArray *synthConfig = SystemConfig("synth", "xbox_headset");
    // synthConfig->FindData("noise_threshold", );
    //  synthConfig->FindData("low_cut", unk20.front().unk0);
    //  synthConfig->FindData("local_gain", unk1c);
    //  synthConfig->FindData("remote_gain", unk2c);
    //  GainEffect::sGain = DbToRatio();
}

MicManagerXbox::~MicManagerXbox() {}

void MicManagerXbox::RequirePushToTalk(bool b, int pad) {
    unk68.Enter();
    if (b) {
        MILO_ASSERT(pad >=0, 0x2c7);
        unk88 = pad;
    } else {
        unk88 = -1;
    }

    unk68.Exit();
}

#pragma endregion MicManagerXbox
