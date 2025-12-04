#include "synth_xbox/Synth.h"
#include "FxSendBitCrush.h"
#include "FxSendChorus.h"
#include "FxSendCompress.h"
#include "FxSendDelay.h"
#include "FxSendDistortion.h"
#include "FxSendEQ.h"
#include "FxSendFlanger.h"
#include "FxSendMeterEffect.h"
#include "FxSendReverb.h"
#include "FxSendWah.h"
#include "Synth.h"
#include "macros.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "stl/_algobase.h"
#include "synth/Synth.h"
#include "synth_xbox/FxSend.h"
#include "synth_xbox/Mic.h"
#include "utl/Std.h"
#include "xdk/xapilibi/xbox.h"
#include "xdk/xaudio2/xaudio2.h"

Synth360 *TheXboxSynth;

Synth360::Synth360()
    : unke8(0), unkec(0), unkf0(0), unkf4(0), unkf8(0), unkfc(0), unk104(true),
      unk105(false), unk138(false), unk13c(0), unk14c(false) {}

BEGIN_HANDLERS(Synth360)
    HANDLE_SUPERCLASS(Synth)
END_HANDLERS

void Synth360::Init() {
    Synth::Init();
    REGISTER_OBJ_FACTORY(FxSendReverb360)
    REGISTER_OBJ_FACTORY(FxSendDelay360)
    REGISTER_OBJ_FACTORY(FxSendCompress360)
    REGISTER_OBJ_FACTORY(FxSendEQ360)
    REGISTER_OBJ_FACTORY(FxSendFlanger360)
    REGISTER_OBJ_FACTORY(FxSendMeterEffect360)
    REGISTER_OBJ_FACTORY(FxSendWah360)
    REGISTER_OBJ_FACTORY(FxSendBitCrush360)
    REGISTER_OBJ_FACTORY(FxSendDistortion360)
    REGISTER_OBJ_FACTORY(FxSendChorus360)
}

Mic *Synth360::GetMic(int index) { return mMics[index]; }

bool Synth360::HasPendingVoices() { return Voice::HasPendingVoices(); }

bool Synth360::DidMicsChange() const {
    if (mMics.empty())
        return false;
    else {
        MicManagerXbox *x = MicManagerXbox::GetInstance();
        return x->unk30;
    }
}

void Synth360::ResetMicsChanged() {
    if (!mMics.empty()) {
        MicManagerXbox *x = MicManagerXbox::GetInstance();
        x->unk30 = false;
    }
}

void Synth360::CaptureMic(int micID) {
    MILO_ASSERT_RANGE(micID, 0, mMics.size(), 0x350);
    MILO_ASSERT(!mMics[micID]->IsInUse(), 0x351);
    mMics[micID]->MarkAsInUse(true);
}

void Synth360::ReleaseAllMics() {
    for (int i = 0; i < mMics.size(); i++) {
        mMics[i]->MarkAsInUse(false);
    }
}

void Synth360::AddFxSend(FxSend360 *fx) { unk140.push_back(fx); }

bool Synth360::IsMicConnected(int i) const {
    if (i < 0 || i >= mMics.size())
        return false;
    else {
        return mMics[i]->GetType() != 0;
    }
}

void Synth360::RequirePushToTalk(bool b, int i) {
    if (!mMics.empty()) {
        MicManagerXbox::GetInstance()->RequirePushToTalk(b, i);
    }
}

void Synth360::ReleaseMic(int micID) {
    MILO_ASSERT_RANGE(micID, 0, mMics.size(), 0x35b);
    if (!mMics[micID]->IsInUse()) {
        MILO_NOTIFY_ONCE("Releasing a microphone [%d]that was not in use\n", micID);
    }
    mMics[micID]->MarkAsInUse(false);
}

void Synth360::RemoveFxSend(FxSend360 *fx) {
    auto *findFx = std::find(unk140.begin(), unk140.end(), fx);
    if (findFx != unk140.end()) {
        unk140.erase(findFx);
    }
}

IXAudio2SubmixVoice *Synth360::GetHeadsetSubmix(int i) {
    if (!unkdc.empty() && i != -1) {
        return unkdc[i];
    }
    return nullptr;
}

int Synth360::GetNextAvailableMicID() const {
    for (int i = 0; i < mMics.size(); i++) {
        if (!mMics[i]->IsInUse() && mMics[i]->GetType() != 0)
            return i;
    }
    return -1;
}

void Synth360::SetDolby(bool b1, bool b2) {
    if (b2) {
        unk104 = b1;
        UpdateDolby();
    } else if (unk104 != b1) {
        unk108.Restart();
        unk104 = b1;
        unk105 = true;
    }
}
