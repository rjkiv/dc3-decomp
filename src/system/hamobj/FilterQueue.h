#pragma once
#include "gesture/Skeleton.h"
#include "hamobj/DetectFrame.h"
#include "hamobj/ErrorNode.h"
#include "hamobj/FilterVersion.h"
#include "hamobj/HamMove.h"
#include <vector>

// size 0x34
class FilterQueue {
public:
    FilterQueue();

    bool GetResults(float &seconds, DetectFrame **plf_frames, float plf_min_time_error);
    void EnqueueNewJob(float seconds, float song_speed, MoveMode mode);
    void EnqueueFrame(
        int player,
        float time_error,
        float delta_secs,
        DetectFrame *df,
        const FilterVersion *fv
    );
    bool IsJobFinished() const;
    float LastPollMs() const;
    bool HasJob() const;
    void CancelJob();
    void StartJob();
    void Poll(const SkeletonUpdateData &data);

private:
    // size 0x14
    class FilterInputFrame {
    public:
        int mPlayer; // 0x0
        float mTimeError; // 0x4
        float mDeltaSecs; // 0x8
        DetectFrame *mDetectFrame; // 0xc
        const FilterVersion *mFilterVersion; // 0x10
    };

    // size 0x214
    class FilterOutputFrame {
    public:
        const FilterInputFrame *mInputFrame; // 0x0
        Vector3 mNodeErrors[kMaxNumErrorNodes]; // 0x4
    };

    struct QueuedJob {
        float mSeconds; // 0x0
        MoveMode mMode; // 0x4
        float mSongSpeed; // 0x8
        std::vector<FilterInputFrame> mQueuedFrames; // 0xc
    };

    struct ThreadJob {
        float mSongSpeed; // 0x0
        MoveMode mMode; // 0x4
        std::vector<FilterOutputFrame> mOutputFrames; // 0x8
        bool mIsFinished; // 0x14
    };

    QueuedJob mQueuedJob; // 0x0
    ThreadJob mThreadJob; // 0x18
    float mLastPollMs; // 0x30
};
