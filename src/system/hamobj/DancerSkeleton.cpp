#include "hamobj/DancerSkeleton.h"
#include "gesture/BaseSkeleton.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "os/Debug.h"
#include "utl/BinStream.h"

DancerSkeleton::DancerSkeleton() {
    for (int i = 0; i < kNumBones; i++) {
        mCamBoneLengths[i] = 0;
    }
}

void DancerSkeleton::Write(BinStream &bs) {
    for (int i = 0; i < kNumJoints; i++) {
        bs << mCamJointPositions[i];
        bs << mCamJointDisplacements[i];
    }
    bs << mElapsedMs;
    bs << mTracked;
}

void DancerSkeleton::Read(BinStream &bs) {
    for (int i = 0; i < kNumJoints; i++) {
        bs >> mCamJointPositions[i];
        bs >> mCamJointDisplacements[i];
    }
    bs >> mElapsedMs;
    bs >> mTracked;
}

bool DancerSkeleton::Displacements(
    const SkeletonHistory *history, SkeletonCoordSys cs, int i3, Vector3 *v, int &iref
) const {
    for (int i = 0; i < kNumJoints; i++) {
        if (!Displacement(history, cs, (SkeletonJoint)i, i3, v[i], iref))
            return false;
    }
    return true;
}

void DancerSkeleton::CamJointDisplacements(Vector3 *disps) const {
    memcpy(disps, mCamJointDisplacements, sizeof(mCamJointDisplacements));
}

void DancerSkeleton::CamJointPositions(Vector3 *pos) const {
    memcpy(pos, mCamJointPositions, sizeof(mCamJointPositions));
}

void DancerSkeleton::CamBoneLengths(float *lens) const {
    DancerSkeleton *me = const_cast<DancerSkeleton *>(this);
    for (int i = 0; i < kNumBones; i++) {
        if (mCamBoneLengths[i] == 0) {
            me->mCamBoneLengths[i] =
                BaseSkeleton::BoneLength((SkeletonBone)i, kCoordCamera);
        }
    }
    memcpy(lens, mCamBoneLengths, sizeof(mCamBoneLengths));
}

void DancerSkeleton::Init() {
    memset(mCamJointPositions, 0, sizeof(mCamJointPositions));
    memset(mCamJointDisplacements, 0, sizeof(mCamJointDisplacements));
    memset(mCamBoneLengths, 0, sizeof(mCamBoneLengths));
    mElapsedMs = -1;
    mTracked = false;
}

void DancerSkeleton::Set(const BaseSkeleton &skeleton) {
    for (int i = 0; i < kNumJoints; i++) {
        skeleton.JointPos(kCoordCamera, (SkeletonJoint)i, mCamJointPositions[i]);
    }
    mTracked = true;
    memset(mCamBoneLengths, 0, sizeof(mCamBoneLengths));
}

void DancerSkeleton::SetDisplacementElapsedMs(int ms) { mElapsedMs = ms; }

void DancerSkeleton::JointPos(SkeletonCoordSys cs, SkeletonJoint joint, Vector3 &pos)
    const {
    MILO_ASSERT((0) <= (joint) && (joint) < (kNumJoints), 0x1C);
    if (cs == kCoordCamera) {
        pos = mCamJointPositions[joint];
    } else {
        Transform xfm;
        CameraToPlayerXfm(cs, xfm);
        MultiplyTranspose(mCamJointPositions[joint], xfm, pos);
    }
}

bool DancerSkeleton::Displacement(
    const SkeletonHistory *history,
    SkeletonCoordSys cs,
    SkeletonJoint joint,
    int,
    Vector3 &disp,
    int &elapsedMs
) const {
    MILO_ASSERT(cs == kCoordCamera, 0x2F);
    MILO_ASSERT((0) <= (joint) && (joint) < (kNumJoints), 0x30);
    if (mElapsedMs != -1) {
        disp = mCamJointDisplacements[joint];
        elapsedMs = mElapsedMs;
        return true;
    } else
        return false;
}

float DancerSkeleton::BoneLength(SkeletonBone bone, SkeletonCoordSys cs) const {
    DancerSkeleton *me = const_cast<DancerSkeleton *>(this);
    MILO_ASSERT(cs == kCoordCamera, 0x65);
    if (mCamBoneLengths[bone] == 0) {
        me->mCamBoneLengths[bone] = BaseSkeleton::BoneLength(bone, cs);
    }
    return mCamBoneLengths[bone];
}

const Vector3 &DancerSkeleton::CamJointPos(SkeletonJoint joint) const {
    MILO_ASSERT((0) <= (joint) && (joint) < (kNumJoints), 0x7A);
    return mCamJointPositions[joint];
}

const Vector3 &DancerSkeleton::CamJointDisplacement(SkeletonJoint joint) const {
    MILO_ASSERT((0) <= (joint) && (joint) < (kNumJoints), 0x80);
    return mCamJointDisplacements[joint];
}

void DancerSkeleton::SetCamJointPos(SkeletonJoint joint, const Vector3 &pos) {
    MILO_ASSERT((0) <= (joint) && (joint) < (kNumJoints), 0xB5);
    mCamJointPositions[joint] = pos;
    memset(mCamBoneLengths, 0, sizeof(mCamBoneLengths));
}

void DancerSkeleton::SetCamJointDisplacement(SkeletonJoint joint, const Vector3 &disp) {
    MILO_ASSERT((0) <= (joint) && (joint) < (kNumJoints), 0xBD);
    mCamJointDisplacements[joint] = disp;
}
