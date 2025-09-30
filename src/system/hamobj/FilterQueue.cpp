#include "hamobj/FilterQueue.h"
#include "hamobj/DetectFrame.h"
#include "hamobj/HamMove.h"
#include "os/Debug.h"
#include "utl/Loader.h"

FilterQueue::FilterQueue() : jobFinished(0), lastPollMs(0) {}

bool FilterQueue::GetResults(float &f1, DetectFrame **frames, float f3) {
    jobFinished = false;
    if (qframes.empty()) {
        oframes.clear();
    }
    f1 = unk0;
    MILO_ASSERT(qframes.size() == oframes.size(), 0x42);
    frames[0] = nullptr;
    frames[1] = nullptr;
    for (int i = 0; i < qframes.size(); i++) {
        qframes[i].unkc->AddError(oframes[i].unk4, qframes[i].unk4);
    }
    qframes.clear();
    oframes.clear();
    return true;
}

void FilterQueue::EnqueueNewJob(float f1, float f2, MoveMode mode) {
    if (!qframes.empty()) {
        MILO_NOTIFY("Queuing new job, but there are already queued frames");
        qframes.clear();
    }
    unk0 = f1;
    unk4 = mode;
    unk8 = f2;
}

void FilterQueue::EnqueueFrame(
    int i1, float f2, float f3, DetectFrame *df, const FilterVersion *fv
) {
    FilterInputFrame frame;
    frame.unk0 = i1;
    frame.unk4 = f2;
    frame.unk8 = f3;
    frame.unkc = df;
    frame.unk10 = fv;
    qframes.push_back(frame);
}

bool FilterQueue::IsJobFinished() const { return jobFinished; }
float FilterQueue::LastPollMs() const { return lastPollMs; }
bool FilterQueue::HasJob() const { return !oframes.empty(); }
void FilterQueue::CancelJob() { qframes.clear(); }

void FilterQueue::StartJob() {
    if (!oframes.empty()) {
        if (!TheLoadMgr.EditMode()) {
            MILO_NOTIFY("Starting new job, but there are unprocessed output frames");
        }
        oframes.clear();
    }
    unk18 = unk8;
    jobFinished = false;
    unk1c = unk4;
    int numQFrames = qframes.size();
    oframes.resize(numQFrames);
    for (int i = 0; i < numQFrames; i++) {
        oframes[i].unk0 = qframes[i].unk0;
    }
}
