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
    virtual void
    JointPos(SkeletonCoordSys cs, SkeletonJoint joint, Vector3 &pos) const = 0; // 0x4
    virtual bool Displacement(
        const SkeletonHistory *history,
        SkeletonCoordSys cs,
        SkeletonJoint j,
        int desired_ms,
        Vector3 &disp,
        int &actual_ms
    ) const = 0; // 0x8
    virtual bool Displacements(
        const SkeletonHistory *history,
        SkeletonCoordSys cs,
        int desired_ms,
        Vector3 *displacements,
        int &actual_ms
    ) const = 0; // 0xc
    virtual JointConfidence JointConf(SkeletonJoint joint) const = 0; // 0x10
    virtual bool IsTracked() const = 0; // 0x14
    virtual int QualityFlags() const = 0; // 0x18
    virtual int ElapsedMs() const = 0; // 0x1c
    virtual void CameraToPlayerXfm(SkeletonCoordSys cs, Transform &xfm) const = 0; // 0x20
    virtual void NormOffset(SkeletonJoint joint, Vector3 &offset) const; // 0x24
    virtual void CamJointPositions(Vector3 *joint_positions) const; // 0x28
    virtual void CamBoneLengths(float *bone_lengths) const; // 0x2c
    virtual float BoneLength(SkeletonBone bone, SkeletonCoordSys cs) const; // 0x30

    void BoneVec(SkeletonBone bone, SkeletonCoordSys cs, Vector3 &vec) const;
    void NormPos(SkeletonCoordSys cs, SkeletonJoint joint, Vector3 &norm_pos) const;

    static SkeletonJoint MirrorJoint(SkeletonJoint joint);
    static void MakeCameraToPlayerXfm(
        SkeletonCoordSys cs,
        Transform &xfm,
        const Vector3 *joints,
        const Vector3 &gravity_normal
    );

    static const BoneJoints sBones[kNumBones];
    static const SkeletonJoint sJointParents[kNumJoints];

protected:
    void CalcNormalizedOffset(SkeletonJoint joint, Vector3 &offset) const;
    void LimbNormPos(
        SkeletonCoordSys cs,
        SkeletonJoint joint,
        bool normalize,
        const Vector3 &in,
        Vector3 &out
    ) const;
};
