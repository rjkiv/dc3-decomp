#include "hamobj/CharCameraInput.h"
#include "char/Character.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/JointUtl.h"
#include "gesture/Skeleton.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Trans.h"

const float CharCameraInput::kDrawScale = 39.370079f;

CharCameraInput::CharCameraInput(Character *c) : mChar(c), unk2430(0) {
    MILO_ASSERT(mChar, 0x18);
    for (int i = 0; i < kNumJoints; i++) {
        const char *name = CharBoneName((SkeletonJoint)i);
        mBoneNames[i] = mChar->Find<RndTransformable>(name, false);
        if (!mBoneNames[i]) {
            MILO_NOTIFY("Could not find %s", name);
        }
    }
    memset(&unk11d8, 0, sizeof(SkeletonFrame));
    unk11d8.unk8.Set(0, 1, 0);
    unk11d8.unk18.Set(0, 0, 0, 0);
    unk11d8.unk0 = 0;
    unk11d8.mElapsedMs = 33;
    for (int i = 0; i < 6; i++) { // literally why is this for loop here
        SkeletonData &data = unk11d8.mSkeletonDatas[i];
        if (i == 0) {
            data.mTracking = kSkeletonTracked;
            data.mQualityFlags = 0;
            for (int j = 0; j < kNumJoints; j++) {
                data.unk284[j] = kSkeletonTracked;
            }
        }
    }
    ResetSkeletonCharOrigin();
}

bool CharCameraInput::NatalToWorld(Transform &world) const {
    world = mNatalXfm;
    return true;
}

const SkeletonFrame *CharCameraInput::PollNewFrame() {
    MILO_ASSERT(mChar, 0x48);
    Transform t;
    Invert(mNatalXfm, t);
    SkeletonData &data = unk11d8.mSkeletonDatas[0];
    for (int i = 0; i < kNumJoints; i++) {
        SkeletonJoint mirrorJoint = BaseSkeleton::MirrorJoint((SkeletonJoint)i);
        RndTransformable *currBone = mBoneNames[i];
        if (currBone) {
            Multiply(currBone->WorldXfm().v, t, data.unk144[mirrorJoint]);
        } else {
            data.unk144[mirrorJoint].Zero();
        }

        if (unk2430) {
            data.unk144[mirrorJoint].x *= -1.0f;
            data.unk144[mirrorJoint].z *= -1.0f;
        }
        data.unk4[mirrorJoint] = data.unk144[mirrorJoint];
    }
    return &unk11d8;
}
