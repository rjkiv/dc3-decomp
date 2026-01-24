#include "gesture/BaseSkeleton.h"
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

void BaseSkeleton::CamJointPositions(Vector3 *positions) const {
    for (int i = 0; i < kNumJoints; i++) {
        JointPos(kCoordCamera, (SkeletonJoint)i, positions[i]);
    }
}

void BaseSkeleton::CamBoneLengths(float *lens) const {
    for (int i = 0; i < kNumBones; i++) {
        lens[i] = BoneLength((SkeletonBone)i, kCoordCamera);
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
    MILO_ASSERT((0) <= (joint) && (joint) < (kNumJoints), 0xC5);
    return gMirrorJoints[joint];
}

void BaseSkeleton::BoneVec(SkeletonBone bone, SkeletonCoordSys cs, Vector3 &vres) const {
    MILO_ASSERT((0) <= (bone) && (bone) < (kNumBones), 0xD1);
    MILO_ASSERT((0) <= (cs) && (cs) < (kNumCoordSys), 0xD2);
    Vector3 v1;
    JointPos(cs, sBones[bone].joint1, v1);
    Vector3 v2;
    JointPos(cs, sBones[bone].joint2, v2);
    Subtract(v2, v1, vres);
}

float BaseSkeleton::BoneLength(SkeletonBone bone, SkeletonCoordSys cs) const {
    Vector3 v;
    BoneVec(bone, cs, v);
    return Length(v);
}

void BaseSkeleton::CalcNormalizedOffset(SkeletonJoint joint, Vector3 &vres) const {
    MILO_ASSERT((0) <= (joint) && (joint) < (kNumJoints), 0x13E);
    vres.Zero();
    SkeletonJoint parent = sJointParents[joint];
    if (parent != kNumJoints) {
        Vector3 v1;
        JointPos(kCoordCamera, joint, v1);
        Vector3 v2;
        JointPos(kCoordCamera, parent, v2);
        Subtract(v1, v2, vres);
        Normalize(vres, vres);
    }
}

void BaseSkeleton::NormOffset(SkeletonJoint joint, Vector3 &v) const {
    CalcNormalizedOffset(joint, v);
}

void BaseSkeleton::NormPos(SkeletonCoordSys cs, SkeletonJoint joint, Vector3 &v) const {
    Vector3 v40;
    JointPos(cs, joint, v40);
    LimbNormPos(cs, joint, true, v40, v);
}
