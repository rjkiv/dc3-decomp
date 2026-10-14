#include "meta/DeJitterPanel.h"
#include "ui/UIPanel.h"
#include "utl/DeJitter.h"

DeJitterPanel::DeJitterPanel() : mFirstPoll(true) {}

DeJitterPanel::~DeJitterPanel() {}

void DeJitterPanel::Enter() {
    mJitter.Reset();
    mFirstPoll = true;
    DeJitterSetter setter(mJitter, 0);
    UIPanel::Enter();
}

void DeJitterPanel::Poll() {
    if (mFirstPoll) {
        mTimer.Restart();
        float f;
        mJitter.NewMs(0, f);
    }
    {
        DeJitterSetter setter(mJitter, mFirstPoll ? nullptr : &mTimer);
        UIPanel::Poll();
    }
    mFirstPoll = false;
}

DeJitterSetter::DeJitterSetter(DeJitter &dj, Timer *t) {
    oldSec = TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo);
    oldDt = TheTaskMgr.DeltaSeconds();
    float f1 = 0.0f;
    float f18 = 0.0f;
    if (t) {
        f1 = dj.NewMs(t->SplitMs(), f18) * 0.001f;
        f18 *= 0.001f;
    }
    TheTaskMgr.SetTimeAndDelta(kTaskSeconds, f1, f18);
}

DeJitterSetter::~DeJitterSetter() {
    TheTaskMgr.SetTimeAndDelta(kTaskSeconds, oldSec, oldDt);
}
