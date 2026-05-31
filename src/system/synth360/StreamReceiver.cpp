#include "synth360/StreamReceiver.h"
#include "os/Debug.h"
#include "synth/ADSR.h"
#include "synth/FxSend.h"
#include "synth/StreamReceiver.h"
#include "synth360/FxSend.h"
#include "utl/MemMgr.h"
#include "utl/Std.h"
#include "xdk/xapilibi/xbox.h"

StreamReceiver *New360Receiver(int i1, int i2, bool b3, int i4) {
    return new StreamReceiver360(i2, i1, b3);
}

StreamReceiver360::StreamReceiver360(int i1, int i2, bool b3)
    : StreamReceiver(i2, b3), unk8038(0), unk803c(0), unk8040(i1), unk8044(i2),
      mVolume(1), mPan(0), mSpeed(1), mSend(0), unk807c(0) {
    int size = i2 << 0xE;
    unk8034 = _MemAllocTemp(size, __FILE__, 0x33, "StreamBuffer", 0);
    unk803c = new Voice(false, 1, false);
    unk803c->SetData(unk8034, size, 0);
    unk803c->SetLoopRegion(0, -1);
    unk803c->SetSampleRate(unk8040);
    if (!mSlipEnabled) {
        unk8038 = unk803c;
    } else {
        unk803c->SetVolume(0);
    }
}

StreamReceiver360::~StreamReceiver360() {
    delete unk803c;
    if (mSlipEnabled) {
        delete unk8038;
    }
    DeleteAll(unk802c);
    MemFree(unk8034);
}

void StreamReceiver360::SetVolume(float volume) {
    mVolume = volume;
    if (unk8038) {
        unk8038->SetVolume(volume);
    }
}

void StreamReceiver360::SetPan(float pan) {
    mPan = pan;
    if (unk8038) {
        unk8038->SetPan(pan);
    }
}

void StreamReceiver360::SetSpeed(float speed) {
    mSpeed = speed;
    unk803c->SetSpeed(speed);
}

void StreamReceiver360::SetADSR(const ADSRImpl &impl) {
    mADSR = impl;
    UpdateADSR();
}

void StreamReceiver360::Tag() {
    unk807c = true;
    if (unk8038) {
        if (unk803c) {
            unk8038->SetUnk50(1);
            unk803c->SetUnk50(2);
        } else if (unk8038) {
            unk8038->SetUnk50(3);
        } else {
            unk803c->SetUnk50(4);
        }
    } else {
        unk803c->SetUnk50(4);
    }
}

void StreamReceiver360::Poll() {
    StreamReceiver::Poll();
    static int v1 = 0;
    static int v2 = 0;
    if (unk803c && unk803c->IsPlaying()) {
        v2++;
    }
    if (unk8038 && unk8038->IsPlaying()) {
        v1++;
    }
    while (!unk802c.empty()) {
        if (unk802c.front()->IsPlaying()) {
            break;
        }
        delete unk802c.front();
        unk802c.pop_front();
    }
}

void StreamReceiver360::SlipStop() {
    MILO_ASSERT(mSlipEnabled, 0xEC);
    if (unk8038) {
        unk8038->Stop(false);
        unk802c.push_back(unk8038);
        unk8038 = nullptr;
    }
}

void StreamReceiver360::SetSlipSpeed(float speed) {
    MILO_ASSERT(mSlipEnabled, 0xFA);
    if (unk8038) {
        unk8038->SetSpeed(speed);
    }
}

void StreamReceiver360::SetFXSend(FxSend *send) {
    mSend = send;
    if (unk8038) {
        unk8038->SetSend(dynamic_cast<FxSend360 *>(send));
    }
}

int StreamReceiver360::GetPlayCursor() { return unk803c->GetAddr(); }

void StreamReceiver360::PauseImpl(bool b1) {
    unk803c->Pause(b1);
    if (mSlipEnabled && unk8038) {
        unk8038->Pause(b1);
    }
}

void StreamReceiver360::PlayImpl() { unk803c->Start(); }

void StreamReceiver360::StartSendImpl(unsigned char *bytes, int size, int i3) {
    XMemCpy((unsigned char *)unk8034 + i3 * 0x4000, bytes, size);
}

void StreamReceiver360::Init() { StreamReceiver::sFactory = New360Receiver; }

void StreamReceiver360::UpdateADSR() {
    if (unk8038) {
        unk8038->SetAttackRate(mADSR.GetAttackRate());
        unk8038->SetReleaseRate(mADSR.GetReleaseRate());
    }
}
