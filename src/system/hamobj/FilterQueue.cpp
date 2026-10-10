#include "hamobj/FilterQueue.h"
#include "gesture/Skeleton.h"
#include "hamobj/DetectFrame.h"
#include "hamobj/ErrorNode.h"
#include "hamobj/FilterVersion.h"
#include "hamobj/HamMove.h"
#include "os/Debug.h"
#include "os/Timer.h"
#include "utl/Loader.h"

FilterQueue::FilterQueue() {
    mThreadJob.mIsFinished = false;
    mLastPollMs = 0;
}

bool FilterQueue::GetResults(
    float &seconds, DetectFrame **plf_frames, float plf_min_time_error
) {
    mThreadJob.mIsFinished = false;
    if (mQueuedJob.mQueuedFrames.empty()) {
        mThreadJob.mOutputFrames.clear();
    }
    seconds = mQueuedJob.mSeconds;
    std::vector<FilterInputFrame> &qframes = mQueuedJob.mQueuedFrames;
    std::vector<FilterOutputFrame> &oframes = mThreadJob.mOutputFrames;
    MILO_ASSERT(qframes.size() == oframes.size(), 0x42);
    plf_frames[0] = plf_frames[1] = nullptr;
    for (int i = 0; i < qframes.size(); i++) {
        auto &qframe = qframes[i];
        qframe.mDetectFrame->AddError(oframes[i].mNodeErrors, qframe.mTimeError);
        if (mQueuedJob.mSeconds > qframe.mDetectFrame->mSeconds
            && qframe.mTimeError > plf_min_time_error) {
            plf_frames[qframe.mPlayer] = qframe.mDetectFrame;
        }
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

void FilterQueue::Poll(const SkeletonUpdateData &data) {
    Timer t;
    t.Start();
    float songSpeed = mThreadJob.mSongSpeed;
    MoveMode mode = mThreadJob.mMode;
    auto &outFrames = mThreadJob.mOutputFrames;
    FOREACH (it, outFrames) {
        FilterOutputFrame &curOutFrame = *it;
        const FilterInputFrame *input = curOutFrame.mInputFrame;
        const FilterVersion *fv = input->mFilterVersion;
        auto *nodes = fv->mNodes;
        const Skeleton *skeleton = data.mPlayerSkeletons[input->mPlayer];
        int numNodes = fv->NumNodes();
        if (skeleton && skeleton->IsTracked()) {
            DetectFrame *df = input->mDetectFrame;
            const MoveFrame *mf = df->mMoveFrame;
            ErrorFrameInput frame_input(
                &data.mHistory, df->mDancerFrame->mSkeleton, *skeleton, songSpeed
            );
            for (int i = 0; i < numNodes; i++) {
                ErrorNode *n = nodes[i];
                if (n->Type() & mf->Flags()) {
                    ErrorNodeInput node_input;
                    input->mFilterVersion->NodeInput(i, df, mode, node_input);
                    n->CalcError(frame_input, node_input, curOutFrame.mNodeErrors[i]);
                } else {
                    curOutFrame.mNodeErrors[i].Set(1, 1, 1);
                }
            }
        } else {
            for (int i = 0; i < numNodes; i++) {
                curOutFrame.mNodeErrors[i].Set(1, 1, 1);
            }
        }
    }
    mThreadJob.mIsFinished = true;
    t.Stop();
    mLastPollMs = t.Ms();
}
