#pragma once
#include "math/Vec.h"
#include "math/Mtx.h"

enum SkeletonBone {
    kBoneHead = 0,
    kBoneCollarRight = 1,
    kBoneArmUpperRight = 2,
    kBoneArmLowerRight = 3,
    kBoneHandRight = 4,
    kBoneCollarLeft = 5,
    kBoneArmUpperLeft = 6,
    kBoneArmLowerLeft = 7,
    kBoneHandLeft = 8,
    kBoneLegUpperRight = 9,
    kBoneLegLowerRight = 10,
    kBoneLegUpperLeft = 11,
    kBoneLegLowerLeft = 12,
    kBoneBackUpper = 13,
    kBoneBackLower = 14,
    kBoneHipRight = 15,
    kBoneHipLeft = 16,
    kBoneFootLeft = 17,
    kBoneFootRight = 18,
    kNumBones = 19
};

enum SkeletonJoint {
    kJointHipCenter = 0,
    kJointSpine = 1,
    kJointShoulderCenter = 2,
    kJointHead = 3,
    kJointShoulderLeft = 4,
    kJointElbowLeft = 5,
    kJointWristLeft = 6,
    kJointHandLeft = 7,
    kJointShoulderRight = 8,
    kJointElbowRight = 9,
    kJointWristRight = 10,
    kJointHandRight = 11,
    kJointHipLeft = 12,
    kJointKneeLeft = 13,
    kJointAnkleLeft = 14,
    kJointHipRight = 15,
    kJointKneeRight = 16,
    kJointAnkleRight = 17,
    kJointFootLeft = 18,
    kJointFootRight = 19,
    kNumJoints = 20
};

enum SkeletonCoordSys {
    kCoordCamera = 0,
    kCoordLeftArm = 1,
    kCoordRightArm = 2,
    kCoordLeftLeg = 3,
    kCoordRightLeg = 4,
    kCoordPelvis = 5,
    kNumCoordSys = 6
};

enum SkeletonSide {
    kSkeletonLeft = 0,
    kSkeletonRight = 1,
    kNumSkeletonSides = 2
};

enum JointConfidence {
    kConfidenceNotTracked = 0,
    kConfidenceInferred = 1,
    kConfidenceTracked = 2
};

class SkeletonHistory;

struct BoneJoints {
    SkeletonBone mBone; // 0x0
    SkeletonJoint mStartJoint; // 0x4
    SkeletonJoint mEndJoint; // 0x8
};

class BaseSkeleton {
public:
    virtual ~BaseSkeleton() {} // 0x0
    virtual void JointPos(SkeletonCoordSys, SkeletonJoint, Vector3 &) const = 0; // 0x4
    virtual bool Displacement(
        const SkeletonHistory *, SkeletonCoordSys, SkeletonJoint, int, Vector3 &, int &
    ) const = 0; // 0x8
    virtual bool Displacements(
        const SkeletonHistory *, SkeletonCoordSys, int, Vector3 *, int &
    ) const = 0; // 0xc
    virtual JointConfidence JointConf(SkeletonJoint) const = 0; // 0x10
    virtual bool IsTracked() const = 0; // 0x14
    virtual int QualityFlags() const = 0; // 0x18
    virtual int ElapsedMs() const = 0; // 0x1c
    virtual void CameraToPlayerXfm(SkeletonCoordSys, Transform &) const = 0; // 0x20
    virtual void NormOffset(SkeletonJoint, Vector3 &) const; // 0x24
    virtual void CamJointPositions(Vector3 *) const; // 0x28
    virtual void CamBoneLengths(float *) const; // 0x2c
    virtual float BoneLength(SkeletonBone, SkeletonCoordSys) const; // 0x30

    void BoneVec(SkeletonBone, SkeletonCoordSys, Vector3 &) const;
    void NormPos(SkeletonCoordSys, SkeletonJoint, Vector3 &) const;

    static SkeletonJoint MirrorJoint(SkeletonJoint);
    static void
    MakeCameraToPlayerXfm(SkeletonCoordSys, Transform &, const Vector3 *, const Vector3 &);

    static const BoneJoints sBones[kNumBones];
    static const SkeletonJoint sJointParents[kNumJoints];

protected:
    void CalcNormalizedOffset(SkeletonJoint, Vector3 &) const;
    void
    LimbNormPos(SkeletonCoordSys, SkeletonJoint, bool, const Vector3 &, Vector3 &) const;
};
