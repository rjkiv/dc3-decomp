#include "meta_ham/CalibrationPanel.h"
#include "macros.h"
#include "meta_ham/HamPanel.h"
#include "meta_ham/ProfileMgr.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Dir.h"
#include "rndobj/Group.h"
#include "synth/Stream.h"
#include "synth/Synth.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "ui/UIPanel.h"
#include "ui/PanelDir.h"
#include "utl/Symbol.h"
#include <cmath>

#pragma region CalibrationOffsetProvider

CalibrationOffsetProvider::CalibrationOffsetProvider(UIPanel *up) {
    SetName("calibration_offset_provider", ObjectDir::Main());
    mCalibrationPanel = up;
}

BEGIN_HANDLERS(CalibrationOffsetProvider)
    HANDLE_EXPR(get_offset, GetOffset(_msg->Int(2)))
END_HANDLERS

void CalibrationOffsetProvider::Text(
    int, int data, UIListLabel *uiListLabel, UILabel *uiLabel
) const {
    MILO_ASSERT_RANGE(data, 0, mOffsets.size(), 0xbf);
    if (uiListLabel->Matches("offset")) {
        static Symbol cal_offset("cal_offset");
        uiLabel->SetTokenFmt(cal_offset, mOffsets[data]);
    } else if (uiListLabel->Matches("label")) {
        uiLabel->SetTextToken(gNullStr); // why
        static Symbol cal_default("cal_default");
        if (mOffsets[data] != 0) {
            uiLabel->SetTextToken(gNullStr);
        } else {
            uiLabel->SetTextToken(cal_default);
        }
    } else if (uiListLabel->Matches("check")) {
        static Symbol chosen_offset("chosen_offset");
        const DataNode *node = mCalibrationPanel->Property(chosen_offset, true);
        int f = node->Float();
        if (mOffsets[data] == f) {
            uiLabel->SetIcon('b');
        } else
            uiLabel->SetTextToken(gNullStr);
    }
}

void CalibrationOffsetProvider::InitData(RndDir *) {
    mOffsets.clear();
    for (int i = 0; i <= 250; i += 10) {
        mOffsets.push_back(i);
    }
}

int CalibrationOffsetProvider::GetOffset(int selectedPos) {
    MILO_ASSERT_RANGE(selectedPos, 0, mOffsets.size(), 0xae);
    int size = mOffsets.size();
    if (selectedPos <= size) {
        size = (selectedPos >= 0) ? selectedPos : 0;
    }
    return mOffsets[size];
}

#pragma endregion
#pragma region CalibrationPanel

CalibrationPanel::CalibrationPanel()
    : mOffsetProvider(this), mCycleTime(500.0f), mVolDb(-6.0f), mStream(),
      mStartASAP(false) {}

CalibrationPanel::~CalibrationPanel() { mOffsetProvider.Cleanup(); }

BEGIN_HANDLERS(CalibrationPanel)
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS

void CalibrationPanel::Enter() {
    UIPanel::Enter();
    InitializeContent();
    StartAudio();
}

void CalibrationPanel::Exit() {
    UIPanel::Exit();
    mStartASAP = false;
    if (mStream)
        mStream->Stop();
}

void CalibrationPanel::Poll() {
    UIPanel::Poll();
    UpdateAnimation();
    UpdateStream();
}

void CalibrationPanel::InitializeContent() {
    if (mStream)
        RELEASE(mStream);
    mStream = TheSynth->NewStream(
        "sfx/samples/shell/sync_clap", mCycleTime * 0.5f, 2.0f, false
    );
    MILO_ASSERT(mStream, 0x36);
    if (mStream) {
        mStream->SetJump(mCycleTime, 0, 0);
        mStream->SetVolume(mVolDb);
    }
}

void CalibrationPanel::StartAudio() {
    mStream->SetVolume(mVolDb);

    if (0 < mStream->GetFilePos())
        mStream->Resync(0);

    if (mStream->IsReady())
        mStream->Play();
    else
        mStartASAP = true;
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
    if (mStream && mStartASAP && mStream->IsReady()) {
        mStream->Play();
        mStartASAP = false;
    }
}

void CalibrationPanel::UpdateAnimation() {
    float f3 = fmod(mCycleTime * 0.5f + GetAudioTimeMs(), mCycleTime);
    f3 = LoadedDir()->Find<RndGroup>("tick.grp")->EndFrame() * (f3 / mCycleTime);
    float f2 = fmod(f3, LoadedDir()->Find<RndGroup>("tick.grp")->EndFrame());
    LoadedDir()->Find<RndGroup>("tick.grp")->SetFrame(f2, 1);
}

#pragma endregion
