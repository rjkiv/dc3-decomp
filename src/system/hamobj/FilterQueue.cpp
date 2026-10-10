#include "hamobj/FilterQueue.h"
#include "hamobj/DetectFrame.h"
#include "hamobj/HamMove.h"
#include "os/Debug.h"
#include "utl/Loader.h"

FilterQueue::FilterQueue() {
    mThreadJob.mIsFinished = false;
    mLastPollMs = 0;
}

bool FilterQueue::GetResults(
    float &seconds, DetectFrame **plf_frames, float plf_min_time_error
) {
    mThreadJob.mIsFinished = false;
    std::vector<FilterInputFrame> &qframes = mQueuedJob.mQueuedFrames;
    std::vector<FilterOutputFrame> &oframes = mThreadJob.mOutputFrames;
    if (qframes.empty()) {
        oframes.clear();
    }
    seconds = mQueuedJob.mSeconds;
    MILO_ASSERT(qframes.size() == oframes.size(), 0x42);
    plf_frames[0] = nullptr;
    plf_frames[1] = nullptr;
    for (int i = 0; i < qframes.size(); i++) {
        qframes[i].mDetectFrame->AddError(oframes[i].mNodeErrors, qframes[i].mTimeError);
    }
    qframes.clear();
    oframes.clear();
    return true;
}

void FilterQueue::EnqueueNewJob(float seconds, float song_speed, MoveMode mode) {
    std::vector<FilterInputFrame> &qframes = mQueuedJob.mQueuedFrames;
    if (!qframes.empty()) {
        MILO_NOTIFY("Queuing new job, but there are already queued frames");
        qframes.clear();
    }
    mQueuedJob.mSeconds = seconds;
    mQueuedJob.mMode = mode;
    mQueuedJob.mSongSpeed = song_speed;
}

void FilterQueue::EnqueueFrame(
    int player,
    float time_error,
    float delta_secs,
    DetectFrame *df,
    const FilterVersion *fv
) {
    FilterInputFrame frame;
    frame.mPlayer = player;
    frame.mTimeError = time_error;
    frame.mDeltaSecs = delta_secs;
    frame.mDetectFrame = df;
    frame.mFilterVersion = fv;
    mQueuedJob.mQueuedFrames.push_back(frame);
}

bool FilterQueue::IsJobFinished() const { return mThreadJob.mIsFinished; }
float FilterQueue::LastPollMs() const { return mLastPollMs; }
bool FilterQueue::HasJob() const { return !mThreadJob.mOutputFrames.empty(); }
void FilterQueue::CancelJob() { mQueuedJob.mQueuedFrames.clear(); }

void FilterQueue::StartJob() {
    if (!mThreadJob.mOutputFrames.empty()) {
        if (!TheLoadMgr.EditMode()) {
            MILO_NOTIFY("Starting new job, but there are unprocessed output frames");
        }
        mThreadJob.mOutputFrames.clear();
    }
    mThreadJob.mSongSpeed = mQueuedJob.mSongSpeed;
    mThreadJob.mIsFinished = false;
    mThreadJob.mMode = mQueuedJob.mMode;
    int numQFrames = mQueuedJob.mQueuedFrames.size();
    mThreadJob.mOutputFrames.resize(numQFrames);
    for (int i = 0; i < numQFrames; i++) {
        mThreadJob.mOutputFrames[i].mInputFrame = &mQueuedJob.mQueuedFrames[i];
    }
}
