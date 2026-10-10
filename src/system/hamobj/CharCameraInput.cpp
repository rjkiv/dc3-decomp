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
    unk11d8.mUpVector.Set(0, 1, 0);
    unk11d8.mFloorPlane.Set(0, 0, 0, 0);
    unk11d8.mFrameNumber = 0;
    unk11d8.mElapsedMs = 33;
    for (int i = 0; i < 6; i++) { // literally why is this for loop here
        SkeletonData &data = unk11d8.mSkeletonDatas[i];
        if (i == 0) {
            data.mTracking = kSkeletonTracked;
            data.mQualityFlags = 0;
            for (int j = 0; j < kNumJoints; j++) {
                data.mJointConfidences[j] = kConfidenceTracked;
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
            Multiply(currBone->WorldXfm().v, t, data.mJointPositions[mirrorJoint]);
        } else {
            data.mJointPositions[mirrorJoint].Zero();
        }

        if (unk2430) {
            data.mJointPositions[mirrorJoint].x *= -1.0f;
            data.mJointPositions[mirrorJoint].z *= -1.0f;
        }
        data.mRawJointPositions[mirrorJoint] = data.mJointPositions[mirrorJoint];
    }
    return &unk11d8;
}

void CharCameraInput::ResetSkeletonCharOrigin() {
    mNatalXfm.Reset();
    float scale = DrawScale();
    mNatalXfm.m.Set(-scale, 0, 0, 0, 0, scale, 0, -scale, 0);
    Vector3 charV = mChar->WorldXfm().v;
    charV.y += DrawScale() * 2;
    charV.z += DrawScale();
    mNatalXfm.v = charV;
}
