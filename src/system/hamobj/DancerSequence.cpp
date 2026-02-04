#include "hamobj/DancerSequence.h"
#include "DancerSkeleton.h"
#include "gesture/BaseSkeleton.h"
#include "hamobj/MoveDir.h"
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

INIT_REVS(8, 0)

BEGIN_LOADS(DancerSequence)
    LOAD_REVS(bs)
    ASSERT_REVS(8, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndAnimatable)
    int numFrames;
    d >> numFrames;
    mDancerFrames.resize(numFrames);
    for (int i = 0; i < numFrames; i++) {
        DancerFrame &curFrame = mDancerFrames[i];
        if (d.rev < 1) {
            int x;
            d >> x;
            curFrame.unk0 = curFrame.mMoveFrameIdx = -1;
        } else if (d.rev < 7) {
            int x, y;
            d >> x;
            d >> y;
            curFrame.unk0 = x;
            curFrame.mMoveFrameIdx = y;
        } else {
            d >> curFrame.unk0;
            d >> curFrame.mMoveFrameIdx;
        }
        DancerSkeleton &skeleton = curFrame.mSkeleton;
        if (d.rev < 7) {
            int skeletonRev = 5;
            if (d.rev < 2) {
                skeletonRev = 0;
                skeleton.SetDisplacementElapsedMs(-1);
            } else if (d.rev < 3) {
                skeletonRev = 1;
                skeleton.SetDisplacementElapsedMs(-1);
            } else if (d.rev < 4) {
                skeletonRev = 2;
                skeleton.SetDisplacementElapsedMs(-1);
            } else if (d.rev < 5) {
                skeletonRev = 3;
                skeleton.SetDisplacementElapsedMs(-1);
            } else if (d.rev < 6) {
                skeletonRev = 4;
                int ms;
                d >> ms;
                skeleton.SetDisplacementElapsedMs(ms);
            }
            if (skeletonRev < 3) {
                bool b;
                d >> b;
                int x;
                d >> x;
            }
            for (int i = 0; i < kNumJoints; i++) {
                int count = 6;
                if (skeletonRev < 1) {
                    count = 4;
                } else if (skeletonRev < 2) {
                    count = 9;
                }
                for (int j = 0; j < count; j++) {
                    if (j >= 6) {
                        Vector3 v;
                        d >> v >> v >> v;
                    } else {
                        if (j == 0) {
                            Vector3 pos, disp;
                            d >> pos;
                            d >> disp;
                            skeleton.SetCamJointPos((SkeletonJoint)i, pos);
                            skeleton.SetCamJointDisplacement((SkeletonJoint)i, disp);
                        } else {
                            Vector3 v1, v2;
                            d >> v1 >> v2;
                        }
                        if (skeletonRev < 4) {
                            Vector3 v;
                            d >> v;
                        }
                    }
                }
                if (skeletonRev < 3) {
                    std::vector<float> floats;
                    d >> floats;
                    int x;
                    d >> x;
                } else if (skeletonRev > 4) {
                    int x;
                    d >> x;
                }
            }
            if (skeletonRev < 2) {
                for (int i = 0; i < 2; i++) {
                    for (int j = 0; j < 3; j++) {
                        std::vector<float> floats;
                        d >> floats;
                    }
                }
            }
        } else {
            for (int i = 0; i < kNumJoints; i++) {
                Vector3 pos;
                Vector3 disp;
                d >> pos;
                d >> disp;
                skeleton.SetCamJointPos((SkeletonJoint)i, pos);
                skeleton.SetCamJointDisplacement((SkeletonJoint)i, disp);
                if (d.rev < 8) {
                    int x;
                    d >> x;
                }
            }
            int ms;
            d >> ms;
            skeleton.SetDisplacementElapsedMs(ms);
        }
    }
END_LOADS

void DancerSequence::SetFrame(float frame, float blend) {
    RndAnimatable::SetFrame(frame, blend);
    MoveDir *m = dynamic_cast<MoveDir *>(this->Dir());
    if (m)
        m->SetDancerSequence(this);
}

float DancerSequence::EndFrame() { return mDancerFrames.size() - 1.0f; }

const std::vector<DancerFrame> &DancerSequence::GetDancerFrames() const {
    return mDancerFrames;
}

const DancerSkeleton *DancerSequence::CurSkeleton() const {
    int idx = GetFrame();
    if (idx >= 0 && idx < mDancerFrames.size()) {
        return &mDancerFrames[idx].mSkeleton;
    } else {
        return nullptr;
    }
}
