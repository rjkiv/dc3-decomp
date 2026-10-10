#pragma once
#include "CharCameraInput.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonHistory.h"
#include "hamobj/HamCharacter.h"
#include "math/Mtx.h"

class MocapSkeletonIterator : public SkeletonHistoryArchive, public SkeletonHistory {
public:
    MocapSkeletonIterator(int start_frame, int end_frame);
    ~MocapSkeletonIterator();
    virtual bool PrevSkeleton(const Skeleton &, int, ArchiveSkeleton &, int &) const;

    operator bool();
    void operator++();

    int Frame() const { return mFrame; }

private:
    void Update();

    HamCharacter *mDancer; // 0x4c
    CharCameraInput mCamInput; // 0x50
    int mStartFrame; // 0x24b0
    int mEndFrame; // 0x24b4
    int mFrame; // 0x24b8
    float mPrevFrame; // 0x24bc
    float mSeconds; // 0x24c0 - unused
    Skeleton mSkeleton; // 0x24c4
    float mOrigSeconds; // 0x2f98
    Transform mOrigXfm; // 0x2f9c
};
