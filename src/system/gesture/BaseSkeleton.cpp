#include "gesture/BaseSkeleton.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "os/Debug.h"

const BoneJoints BaseSkeleton::sBones[] = {
    { kBoneHead, kJointHead, kJointShoulderCenter },
    { kBoneCollarRight, kJointShoulderCenter, kJointShoulderRight },
    { kBoneArmUpperRight, kJointShoulderRight, kJointElbowRight },
    { kBoneArmLowerRight, kJointElbowRight, kJointWristRight },
    { kBoneHandRight, kJointWristRight, kJointHandRight },
    { kBoneCollarLeft, kJointShoulderCenter, kJointShoulderLeft },
    { kBoneArmUpperLeft, kJointShoulderLeft, kJointElbowLeft },
    { kBoneArmLowerLeft, kJointElbowLeft, kJointWristLeft },
    { kBoneHandLeft, kJointWristLeft, kJointHandLeft },
    { kBoneLegUpperRight, kJointHipRight, kJointKneeRight },
    { kBoneLegLowerRight, kJointKneeRight, kJointAnkleRight },
    { kBoneLegUpperLeft, kJointHipLeft, kJointKneeLeft },
    { kBoneLegLowerLeft, kJointKneeLeft, kJointAnkleLeft },
    { kBoneBackUpper, kJointShoulderCenter, kJointSpine },
    { kBoneBackLower, kJointSpine, kJointHipCenter },
    { kBoneHipRight, kJointHipRight, kJointHipCenter },
    { kBoneHipLeft, kJointHipCenter, kJointHipLeft },
    { kBoneFootLeft, kJointAnkleLeft, kJointFootLeft },
    { kBoneFootRight, kJointAnkleRight, kJointFootRight }
};

const SkeletonJoint BaseSkeleton::sJointParents[] = {
    kNumJoints,           kJointHipCenter,     kJointSpine,      kJointShoulderCenter,
    kJointShoulderCenter, kJointShoulderLeft,  kJointElbowLeft,  kJointWristLeft,
    kJointShoulderCenter, kJointShoulderRight, kJointElbowRight, kJointWristRight,
    kJointHipCenter,      kJointHipLeft,       kJointKneeLeft,   kJointHipCenter,
    kJointHipRight,       kJointKneeRight,     kJointAnkleLeft,  kJointAnkleRight
};

void BaseSkeleton::CamJointPositions(Vector3 *joint_positions) const {
    for (int i = 0; i < kNumJoints; i++) {
        JointPos(kCoordCamera, (SkeletonJoint)i, joint_positions[i]);
    }
}

void BaseSkeleton::CamBoneLengths(float *bone_lengths) const {
    for (int i = 0; i < kNumBones; i++) {
        bone_lengths[i] = BoneLength((SkeletonBone)i, kCoordCamera);
    }
}

SkeletonJoint gMirrorJoints[kNumJoints] = {
    kJointHipCenter,     kJointSpine,      kJointShoulderCenter, kJointHead,
    kJointShoulderRight, kJointElbowRight, kJointWristRight,     kJointHandRight,
    kJointShoulderLeft,  kJointElbowLeft,  kJointWristLeft,      kJointHandLeft,
    kJointHipRight,      kJointKneeRight,  kJointAnkleRight,     kJointHipLeft,
    kJointKneeLeft,      kJointAnkleLeft,  kJointFootRight,      kJointFootLeft
};

SkeletonJoint BaseSkeleton::MirrorJoint(SkeletonJoint joint) {
    MILO_ASSERT_RANGE(joint, 0, kNumJoints, 0xC5);
    return gMirrorJoints[joint];
}

void BaseSkeleton::BoneVec(SkeletonBone bone, SkeletonCoordSys cs, Vector3 &vec) const {
    MILO_ASSERT_RANGE(bone, 0, kNumBones, 0xD1);
    MILO_ASSERT_RANGE(cs, 0, kNumCoordSys, 0xD2);
    Vector3 v1;
    JointPos(cs, sBones[bone].mStartJoint, v1);
    Vector3 v2;
    JointPos(cs, sBones[bone].mEndJoint, v2);
    Subtract(v2, v1, vec);
}

float BaseSkeleton::BoneLength(SkeletonBone bone, SkeletonCoordSys cs) const {
    Vector3 v;
    BoneVec(bone, cs, v);
    return sqrtf(v.z * v.z + v.y * v.y + v.x * v.x);
}

void BaseSkeleton::CalcNormalizedOffset(SkeletonJoint joint, Vector3 &offset) const {
    MILO_ASSERT_RANGE(joint, 0, kNumJoints, 0x13E);
    offset.Zero();
    SkeletonJoint parent = sJointParents[joint];
    if (parent != kNumJoints) {
        Vector3 v1;
        JointPos(kCoordCamera, joint, v1);
        Vector3 v2;
        JointPos(kCoordCamera, parent, v2);
        Subtract(v1, v2, offset);
        Normalize(offset, offset);
    }
}

void BaseSkeleton::NormOffset(SkeletonJoint joint, Vector3 &offset) const {
    CalcNormalizedOffset(joint, offset);
}

void BaseSkeleton::NormPos(
    SkeletonCoordSys cs, SkeletonJoint joint, Vector3 &norm_pos
) const {
    Vector3 v40;
    JointPos(cs, joint, v40);
    LimbNormPos(cs, joint, true, v40, norm_pos);
}

void BaseSkeleton::LimbNormPos(
    SkeletonCoordSys cs,
    SkeletonJoint joint,
    bool normalize,
    const Vector3 &in,
    Vector3 &out
) const {
    MILO_ASSERT(cs == kCoordLeftArm || cs == kCoordRightArm || cs == kCoordLeftLeg || cs == kCoordRightLeg, 0x10E);

    SkeletonJoint jointRoot, jointMid, jointEnd, jointTip;
    SkeletonBone boneUpper, boneLower, boneEnd;
    if (cs == kCoordLeftArm || cs == kCoordRightArm) {
        bool right = cs == kCoordRightArm;
        jointRoot = right ? kJointShoulderRight : kJointShoulderLeft;
        jointMid = right ? kJointElbowRight : kJointElbowLeft;
        jointEnd = right ? kJointWristRight : kJointWristLeft;
        jointTip = right ? kJointHandRight : kJointHandLeft;
        boneUpper = right ? kBoneArmUpperRight : kBoneArmUpperLeft;
        boneLower = right ? kBoneArmLowerRight : kBoneArmLowerLeft;
        boneEnd = right ? kBoneHandRight : kBoneHandLeft;
    } else {
        bool right = cs == kCoordRightLeg;
        jointRoot = right ? kJointHipRight : kJointHipLeft;
        jointMid = right ? kJointKneeRight : kJointKneeLeft;
        jointEnd = right ? kJointAnkleRight : kJointAnkleLeft;
        jointTip = right ? kJointFootRight : kJointFootLeft;
        boneUpper = right ? kBoneLegUpperRight : kBoneLegUpperLeft;
        boneLower = right ? kBoneLegLowerRight : kBoneLegLowerLeft;
        boneEnd = right ? kBoneFootRight : kBoneFootLeft;
    }
    out = in;
    if (joint != jointRoot) {
        if (joint == jointMid || joint == jointEnd || joint == jointTip) {
            float len = BoneLength(boneUpper, kCoordCamera);
            if (joint == jointEnd || joint == jointTip) {
                len += BoneLength(boneLower, kCoordCamera);
                if (joint == jointTip) {
                    len += BoneLength(boneEnd, kCoordCamera);
                }
            }
            if (normalize && len > 0) {
                len = 1 / len;
            }
            Scale(out, len, out);
        } else {
            MILO_FAIL("Unsupported joint %i", joint);
        }
    }
}

void BaseSkeleton::MakeCameraToPlayerXfm(
    SkeletonCoordSys cs,
    Transform &xfm,
    const Vector3 *joints,
    const Vector3 &gravity_normal
) {
    MILO_ASSERT_RANGE(cs, kCoordLeftArm, kNumCoordSys, 0x14F);
    Vector3 up_vec = gravity_normal;
    Normalize(up_vec, up_vec);
    Vector3 origin;
    Vector3 vsub;
    if (cs == kCoordLeftArm || cs == kCoordRightArm) {
        origin = joints[cs == kCoordLeftArm ? kJointShoulderLeft : kJointShoulderRight];
        Vector3 left_arm = joints[kJointShoulderLeft];
        Vector3 right_arm = joints[kJointShoulderRight];
        if (cs == kCoordLeftArm) {
            right_arm.z = left_arm.z;
        } else {
            left_arm.z = right_arm.z;
        }
        Subtract(right_arm, left_arm, vsub);
    } else if (cs == kCoordLeftLeg || cs == kCoordRightLeg) {
        origin = joints[cs == kCoordLeftLeg ? kJointHipLeft : kJointHipRight];
        Vector3 left_hip = joints[kJointHipLeft];
        Vector3 right_hip = joints[kJointHipRight];
        if (cs == kCoordLeftLeg) {
            right_hip.z = left_hip.z;
        } else {
            left_hip.z = right_hip.z;
        }
        Subtract(right_hip, left_hip, vsub);
    } else if (cs == kCoordPelvis) {
        Vector3 left_hip = joints[kJointHipLeft];
        Vector3 right_hip = joints[kJointHipRight];
        origin = joints[kJointHipCenter];
        Subtract(right_hip, left_hip, vsub);
    }
    Normalize(vsub, vsub);
    Vector3 crossed;
    Cross(up_vec, vsub, crossed);
    Normalize(crossed, crossed);
    xfm.Set(Hmx::Matrix3(vsub, up_vec, crossed), origin);
}
