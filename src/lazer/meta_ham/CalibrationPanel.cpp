#include "meta_ham/CalibrationPanel.h"
#include "macros.h"
#include "meta_ham/HamPanel.h"
#include "meta_ham/ProfileMgr.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Dir.h"
#include "synth/Stream.h"
#include "synth/Synth.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"

#pragma region CalibrationOffsetProvider

CalibrationOffsetProvider::CalibrationOffsetProvider(UIPanel *up) {
    SetName("calibration_offset_provider", ObjectDir::Main());
    unk3c = up;
}

int CalibrationOffsetProvider::GetOffset(int selectedPos) {
    MILO_ASSERT_RANGE(selectedPos, 0, mOffsets.size(), 0xae);
    int size = mOffsets.size();
    if (selectedPos <= size) {
        size = (selectedPos >= 0) ? selectedPos : 0;
    }
    return mOffsets[size];
}

void CalibrationOffsetProvider::InitData(RndDir *) {
    mOffsets.clear();
    for (int i = 0; i <= 250; i += 10) {
        mOffsets.push_back(i);
    }
}

void CalibrationOffsetProvider::Text(
    int, int data, UIListLabel *uiListLabel, UILabel *uiLabel
) const {
    MILO_ASSERT_RANGE(data, 0, mOffsets.size(), 0xbf);
    if (uiListLabel->Matches("offset")) {
        static Symbol cal_offset("cal_offset");
        uiLabel->SetTokenFmt(cal_offset, mOffsets[data]);
    } else if (uiListLabel->Matches("label")) {
        static Symbol cal_default("cal_default");
    } else if (uiListLabel->Matches("check")) {
        static Symbol chosen_offset("chosen_offset");
    }
}

BEGIN_HANDLERS(CalibrationOffsetProvider)
    HANDLE_ACTION(get_offset, GetOffset(_msg->Int(2)))
END_HANDLERS

#pragma endregion CalibrationOffsetProvider
#pragma region CalibrationPanel

CalibrationPanel::CalibrationPanel()
    : unk3c(this), unk7c(500.0f), mVolume(-6.0f), mStream(), unk88(false) {}

CalibrationPanel::~CalibrationPanel() { unk3c.mOffsets.clear(); }

void CalibrationPanel::Poll() {
    UIPanel::Poll();
    UpdateAnimation();
    UpdateStream();
}

void CalibrationPanel::Enter() {
    UIPanel::Enter();
    InitializeContent();
    StartAudio();
}

void CalibrationPanel::Exit() {
    UIPanel::Exit();
    unk88 = false;
    if (mStream)
        mStream->Stop();
}

void CalibrationPanel::InitializeContent() {
    if (mStream)
        RELEASE(mStream);
    mStream =
        TheSynth->NewStream("sfx/samples/shell/sync_clap", unk7c * 0.5f, 2.0f, false);
    MILO_ASSERT(mStream, 0x36);
    if (mStream) {
        mStream->SetJump(unk7c, 0, 0);
        mStream->SetVolume(mVolume);
    }
}

void CalibrationPanel::StartAudio() {
    mStream->SetVolume(mVolume);

    if (0 < mStream->GetFilePos())
        mStream->Resync(0);

    if (mStream->IsReady())
        mStream->Play();
    else
        unk88 = true;
}

float CalibrationPanel::GetAudioTimeMs() const {
    if (!mStream) {
        return 0;
    } else {
        float f1 = TheProfileMgr.GetSongToTaskMgrMsRaw();
        float f2 = mStream->GetTime();
        return f1 + f2;
    }
}

void CalibrationPanel::UpdateStream() {
    if (mStream && unk88 && mStream->IsReady()) {
        mStream->Play();
        unk88 = false;
    }
}

BEGIN_HANDLERS(CalibrationPanel)
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS

#pragma endregion CalibrationPanel
