#include "synth360/StreamReceiver.h"
#include "math/Utl.h"
#include "os/Debug.h"
#include "synth/ADSR.h"
#include "synth/FxSend.h"
#include "synth/StreamReceiver.h"
#include "synth360/FxSend.h"
#include "synth360/Voice.h"
#include "utl/MemMgr.h"
#include "utl/Std.h"
#include "xdk/xapilibi/xbox.h"

StreamReceiver *New360Receiver(int i1, int i2, bool b3, int i4) {
    return new StreamReceiver360(i2, i1, b3);
}

StreamReceiver360::StreamReceiver360(int i1, int i2, bool b3)
    : StreamReceiver(i2, b3), mVoice(0), mRefVoice(0), mSampleRate(i1), mNumBuffers(i2),
      mVolume(1), mPan(0), mSpeed(1), mFxSend(0), mTagged(0) {
    int size = i2 << 0xE;
    mVoiceBuffer = (char *)_MemAllocTemp(size, __FILE__, 0x33, "StreamBuffer", 0);
    mRefVoice = new Voice(false, 1, false);
    mRefVoice->SetData(mVoiceBuffer, size, 0);
    mRefVoice->SetLoopRegion(0, -1);
    mRefVoice->SetSampleRate(mSampleRate);
    if (!mSlipEnabled) {
        mVoice = mRefVoice;
    } else {
        mRefVoice->SetVolume(0);
    }
}

StreamReceiver360::~StreamReceiver360() {
    delete mRefVoice;
    if (mSlipEnabled) {
        delete mVoice;
    }
    DeleteAll(mOldVoices);
    MemFree(mVoiceBuffer);
}

void StreamReceiver360::SetVolume(float volume) {
    mVolume = volume;
    if (mVoice) {
        mVoice->SetVolume(volume);
    }
}

void StreamReceiver360::SetPan(float pan) {
    mPan = pan;
    if (mVoice) {
        mVoice->SetPan(pan);
    }
}

void StreamReceiver360::SetSpeed(float speed) {
    mSpeed = speed;
    mRefVoice->SetSpeed(speed);
}

void StreamReceiver360::SetADSR(const ADSRImpl &impl) {
    mADSR = impl;
    UpdateADSR();
}

void StreamReceiver360::SetSlipOffset(float off) {
    MILO_ASSERT(mSlipEnabled, 197);
    SlipStop();
    mVoice = new Voice(0, 1, 0);
    if (mTagged) {
        Tag();
    }
    mVoice->SetData(mVoiceBuffer, mNumBuffers * 16384, 0);
    mVoice->SetLoopRegion(0, -1);
    mVoice->SetSampleRate(mSampleRate);
    mVoice->SetStartSamp(GetPlayCursor());
    mVoice->SetVolume(mVolume);
    mVoice->SetPan(mPan);
    mVoice->SetSpeed(mSpeed);
    UpdateADSR();
    SetFXSend(mFxSend);
    mVoice->Start();
}

void StreamReceiver360::Tag() {
    mTagged = true;
    if (mVoice) {
        if (mRefVoice) {
            mVoice->SetTag(1);
            mRefVoice->SetTag(2);
        } else if (mVoice) {
            mVoice->SetTag(3);
        } else {
            mRefVoice->SetTag(4);
        }
    }

    if (mRefVoice)
        mRefVoice->SetTag(4);
}

void StreamReceiver360::Poll() {
    StreamReceiver::Poll();
    static int v1 = 0;
    static int v2 = 0;
    if (mRefVoice && mRefVoice->IsPlaying()) {
        v2++;
    }
    if (mVoice && mVoice->IsPlaying()) {
        v1++;
    }
    while (!mOldVoices.empty()) {
        if (mOldVoices.front()->IsPlaying()) {
            break;
        }
        delete mOldVoices.front();
        mOldVoices.pop_front();
    }
}

void StreamReceiver360::SlipStop() {
    MILO_ASSERT(mSlipEnabled, 0xEC);
    if (mVoice) {
        mVoice->Stop(false);
        mOldVoices.push_back(mVoice);
        mVoice = nullptr;
    }
}

void StreamReceiver360::SetSlipSpeed(float speed) {
    MILO_ASSERT(mSlipEnabled, 0xFA);
    if (mVoice) {
        mVoice->SetSpeed(speed);
    }
}

void StreamReceiver360::SetFXSend(FxSend *send) {
    mFxSend = send;
    if (mVoice) {
        mVoice->SetSend(dynamic_cast<FxSend360 *>(send));
    }
}

int StreamReceiver360::GetPlayCursor() { return mRefVoice->GetAddr(); }

void StreamReceiver360::PauseImpl(bool b1) {
    mRefVoice->Pause(b1);
    if (mSlipEnabled && mVoice) {
        mVoice->Pause(b1);
    }
}

void StreamReceiver360::PlayImpl() { mRefVoice->Start(); }

void StreamReceiver360::StartSendImpl(unsigned char *bytes, int size, int i3) {
    XMemCpy((unsigned char *)mVoiceBuffer + i3 * 0x4000, bytes, size);
}

void StreamReceiver360::Init() { StreamReceiver::sFactory = New360Receiver; }

void StreamReceiver360::UpdateADSR() {
    if (mVoice) {
        mVoice->SetAttackRate(mADSR.GetAttackRate());
        mVoice->SetReleaseRate(mADSR.GetReleaseRate());
    }
}

float StreamReceiver360::GetSlipOffset() {
    MILO_ASSERT(mSlipEnabled, 0x102);
    if (mVoice) {
        int addr1 = mRefVoice->GetAddr();
        int addr2 = mVoice->GetAddr();
        float val = (float)(mNumBuffers << 0xe) / 2;
        float negativeVal = -val;
        float f6 = Mod((addr2 - addr1) - negativeVal, (val - negativeVal));
        return (((f6 + negativeVal) * 0.5f) / mSampleRate) * 1000.0f;
    } else
        return 0;
}
