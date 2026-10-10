#include "gesture/ArchiveSkeleton.h"
#include "Skeleton.h"
#include "gesture/BaseSkeleton.h"
#include "os/Debug.h"
#include "math/Mtx.h"

ArchiveSkeleton::ArchiveSkeleton() {
    memset(mJointPoses, 0, sizeof(mJointPoses));
    memset(mJointConfs, 0, sizeof(mJointConfs));
    mElapsedMs = -1;
    mTracked = false;
    mQualityFlags = 0;
}

void ArchiveSkeleton::Set(const Skeleton &skeleton) {
    const TrackedJoint *joints = skeleton.TrackedJoints();
    for (int i = 0; i < kNumJoints; i++) {
        mJointPoses[i] = joints[i].mPos[kCoordCamera];
        mJointConfs[i] = joints[i].mConfidence;
    }
    mElapsedMs = skeleton.ElapsedMs();
    mTracked = skeleton.IsTracked();
    mQualityFlags = skeleton.QualityFlags();
}

void ArchiveSkeleton::JointPos(
    SkeletonCoordSys cs, SkeletonJoint joint, Vector3 &pos
) const {
    MILO_ASSERT((0) <= (joint) && (joint) < (kNumJoints), 0x1E);
    if (cs == kCoordCamera) {
        pos = mJointPoses[joint];
    } else {
        Transform xfm;
        CameraToPlayerXfm(cs, xfm);
        MultiplyTranspose(mJointPoses[joint], xfm, pos);
    }
}

JointConfidence ArchiveSkeleton::JointConf(SkeletonJoint joint) const {
    MILO_ASSERT((0) <= (joint) && (joint) < (kNumJoints), 0x2C);
    return mJointConfs[joint];
}

void ArchiveSkeleton::CameraToPlayerXfm(SkeletonCoordSys cs, Transform &xfm) const {
    BaseSkeleton::MakeCameraToPlayerXfm(cs, xfm, mJointPoses, Vector3(0, 1, 0));
}
