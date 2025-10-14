#include "meta/DeJitterPanel.h"
#include "ui/UIPanel.h"
#include "utl/DeJitter.h"

DeJitterPanel::DeJitterPanel() : unkf8(true) {}

DeJitterPanel::~DeJitterPanel() {}

void DeJitterPanel::Enter() {
    unk68.Reset();
    unkf8 = true;
    DeJitterSetter setter(unk68, 0);
    UIPanel::Enter();
}

void DeJitterPanel::Poll() {
    if (unkf8) {
        unk38.Restart();
        float f;
        unk68.NewMs(0, f);
    }
    {
        DeJitterSetter setter(unk68, unkf8 ? nullptr : &unk38);
        UIPanel::Poll();
    }
    unkf8 = false;
}

DeJitterSetter::DeJitterSetter(DeJitter &dj, Timer *t) {
    secs = TheTaskMgr.Seconds(TaskMgr::kRealTime);
    delta_secs = TheTaskMgr.DeltaSeconds();
    float f1 = 0.0f;
    float f18 = 0.0f;
    if (t) {
        f1 = dj.NewMs(t->SplitMs(), f18) * 0.001f;
        f18 *= 0.001f;
    }
    TheTaskMgr.SetTimeAndDelta(kTaskSeconds, f1, f18);
}

DeJitterSetter::~DeJitterSetter() {
    TheTaskMgr.SetTimeAndDelta(kTaskSeconds, secs, delta_secs);
}
