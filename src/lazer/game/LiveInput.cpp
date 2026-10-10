#include "game/LiveInput.h"
#include "game/GamePanel.h"
#include "hamobj/HamAudio.h"
#include "meta_ham/ProfileMgr.h"
#include "obj/Task.h"

LiveInput::LiveInput(HamAudio &audio) : mAudio(audio), mTimeOffset(0) {
    mRealTimer.Restart();
}

float LiveInput::CurrentMs(bool b1) const {
    const_cast<LiveInput *>(this)->mRealTimer.Split();
    float toAdd;
    if (b1) {
        toAdd = const_cast<LiveInput *>(this)->mRealTimer.Ms() + mTimeOffset;
    } else {
        toAdd = mAudio.GetTime();
    }
    return TheGamePanel->DeJitter(GetSongToTaskMgrMs() + toAdd);
}

float LiveInput::GetSongToTaskMgrMs() const {
    return TheProfileMgr.GetSongToTaskMgrMs(kGame);
}

void LiveInput::SetPaused(bool b1) {
    if (b1) {
        mAudio.SetPaused(true);
    } else if (!TheGamePanel->IsGameOver() && mAudio.IsReady()) {
        mAudio.SetPaused(false);
    }
}

void LiveInput::SetTimeOffset() {
    float f1 = TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo) * 1000.0f;
    f1 = f1 - mRealTimer.SplitMs();
    mTimeOffset = f1 - TheProfileMgr.GetSongToTaskMgrMs(kGame);
}

void LiveInput::SetPostWaitJumpOffset(float f1) {
    mRealTimer.Restart();
    mTimeOffset = f1 - mRealTimer.Ms();
}
