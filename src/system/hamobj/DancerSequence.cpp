#include "hamobj/DancerSequence.h"
#include "DancerSkeleton.h"
#include "gesture/BaseSkeleton.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Anim.h"

DancerSequence::DancerSequence() {}

BEGIN_HANDLERS(DancerSequence)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(DancerSequence)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(DancerSequence)
    SAVE_REVS(8, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndAnimatable)
    int numFrames = mDancerFrames.size();
    bs << numFrames;
    for (int i = 0; i < numFrames; i++) {
        const DancerFrame &curFrame = mDancerFrames[i];
        bs << curFrame.unk0;
        bs << curFrame.mMoveFrameIdx;
        const DancerSkeleton &skeleton = curFrame.mSkeleton;
        for (int j = 0; j < kNumJoints; j++) {
            bs << skeleton.CamJointPos((SkeletonJoint)j);
            bs << skeleton.CamJointDisplacement((SkeletonJoint)j);
        }
        bs << skeleton.ElapsedMs();
    }
END_SAVES

BEGIN_COPYS(DancerSequence)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    CREATE_COPY_AS(DancerSequence, seq)
    MILO_ASSERT(seq, 0xF2);
    COPY_MEMBER_FROM(seq, mDancerFrames)
END_COPYS

const std::vector<DancerFrame> &DancerSequence::GetDancerFrames() const {
    return mDancerFrames;
}

float DancerSequence::EndFrame() { return mDancerFrames.size() - 1.0f; }
