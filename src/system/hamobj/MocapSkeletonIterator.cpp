#include "hamobj/MocapSkeletonIterator.h"
#include "ClipPlayer.h"
#include "HamRegulate.h"
#include "hamobj/HamCharacter.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "math/Utl.h"
#include "obj/Task.h"
#include "os/Debug.h"

MocapSkeletonIterator::MocapSkeletonIterator(int start_frame, int end_frame)
    : mDancer(TheHamDirector->GetCharacter(0)), mCamInput(mDancer),
      mStartFrame(start_frame), mEndFrame(end_frame) {
    MILO_ASSERT(TheGameData, 0x16);
    mSkeleton.Init();
    mPrevFrame = -kHugeFloat;
    mFrame = mStartFrame;
    mOrigSeconds = TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo);
    HamCharacter *c = mDancer;
    if (mDancer) {
        mOrigXfm = mDancer->LocalXfm();
        c->Enter();
        mDancer->DirtyLocalXfm().Reset();
        mDancer->Teleport(nullptr);
        Update();
        mCamInput.ResetSkeletonCharOrigin();
    }
}

MocapSkeletonIterator::~MocapSkeletonIterator() {
    if (mDancer) {
        mDancer->DirtyLocalXfm().Reset();
        mDancer->Teleport(nullptr);
        mDancer->SetLocalXfm(mOrigXfm);
    }
    TheTaskMgr.SetSeconds(mOrigSeconds, true);
}

MocapSkeletonIterator::operator bool() { return mDancer && mFrame < mEndFrame; }

void MocapSkeletonIterator::operator++() {
    mFrame++;
    Update();
}

bool MocapSkeletonIterator::PrevSkeleton(
    const Skeleton &s, int i2, ArchiveSkeleton &as, int &i3
) const {
    return PrevFromArchive(*this, s, i2, as, i3);
}

void MocapSkeletonIterator::Update() {
    MILO_ASSERT(mDancer, 0x55);
    TheTaskMgr.SetSeconds(mFrame / 30.0f, (mStartFrame - mFrame) == 0);
    ClipPlayer player(0);
    if (player.Init(0)) {
        player.PlayAnims(mDancer, mFrame, mPrevFrame, 0);
        mPrevFrame = mFrame;
    } else {
        MILO_NOTIFY(
            "Failed to init ClipPlayer for %s!", PathName(TheHamDirector->ClipDir())
        );
    }
    HamRegulate *reg = mDancer->Regulator();
    reg->SetWaypoint(nullptr);
    mDancer->Poll();
    if (mSkeleton.IsTracked()) {
        AddToHistory(0, mSkeleton);
    }
    mCamInput.PollTracking();
    const SkeletonFrame *frame_data = mCamInput.NewFrame();
    MILO_ASSERT(frame_data, 0x6F);
    MILO_ASSERT(frame_data->mElapsedMs == 33, 0x70);
    MILO_ASSERT(frame_data->mSkeletonDatas[0].mTracking == kSkeletonTracked, 0x71);
    mSkeleton.Poll(0, *frame_data);
}
