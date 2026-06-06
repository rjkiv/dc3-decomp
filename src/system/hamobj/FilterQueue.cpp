#include "hamobj/FilterQueue.h"
#include "hamobj/DetectFrame.h"
#include "hamobj/HamMove.h"
#include "os/Debug.h"
#include "utl/Loader.h"

FilterQueue::FilterQueue() : jobFinished(0), lastPollMs(0) {}

bool FilterQueue::GetResults(float &f1, DetectFrame **frames, float f3) {
    jobFinished = false;
    std::vector<FilterInputFrame> &qframes = mQueuedJob.frames;
    std::vector<FilterOutputFrame> &oframes = mOutput.frames;
    if (qframes.empty()) {
        oframes.clear();
    }
    f1 = mQueuedJob.unk0;
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
    std::vector<FilterInputFrame> &qframes = mQueuedJob.frames;
    if (!qframes.empty()) {
        MILO_NOTIFY("Queuing new job, but there are already queued frames");
        qframes.clear();
    }
    mQueuedJob.unk0 = f1;
    mQueuedJob.unk4 = mode;
    mQueuedJob.unk8 = f2;
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
    mQueuedJob.frames.push_back(frame);
}

bool FilterQueue::IsJobFinished() const { return jobFinished; }
float FilterQueue::LastPollMs() const { return lastPollMs; }
bool FilterQueue::HasJob() const { return !mOutput.frames.empty(); }
void FilterQueue::CancelJob() { mQueuedJob.frames.clear(); }

void FilterQueue::StartJob() {
    if (!mOutput.frames.empty()) {
        if (!TheLoadMgr.EditMode()) {
            MILO_NOTIFY("Starting new job, but there are unprocessed output frames");
        }
        mOutput.frames.clear();
    }
    mOutput.unk0 = mQueuedJob.unk8;
    jobFinished = false;
    mOutput.unk4 = mQueuedJob.unk4;
    int numQFrames = mQueuedJob.frames.size();
    mOutput.frames.resize(numQFrames);
    for (int i = 0; i < numQFrames; i++) {
        mOutput.frames[i].unk0 = reinterpret_cast<int>(&mQueuedJob.frames[i].unk0);
    }
}
