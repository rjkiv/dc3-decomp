#include "gesture/JointUtl.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "os/Debug.h"
#include "utl/Str.h"

bool IsSkeletonBone(const char *name) {
    static const char *sBoneNames[] = {
        "bone_spine1.mesh",     "bone_spine2.mesh",
        "bone_neck.mesh",       "bone_head.mesh",
        "bone_L-upperArm.mesh", "bone_L-foreArm.mesh",
        "bone_L-hand.mesh",     "bone_L-middlefinger03.mesh",
        "bone_R-upperArm.mesh", "bone_R-foreArm.mesh",
        "bone_R-hand.mesh",     "bone_R-middlefinger03.mesh",
        "bone_L-thigh.mesh",    "bone_L-knee.mesh",
        "bone_L-ankle.mesh",    "bone_R-thigh.mesh",
        "bone_R-knee.mesh",     "bone_R-ankle.mesh",
        "bone_L-toe.mesh",      "bone_R-toe.mesh"
    };
    int numBoneNames = DIM(sBoneNames);
    for (int i = 0; i < numBoneNames; i++) {
        if (streq(sBoneNames[i], name))
            return true;
    }
    return false;
}

const char *JointName(SkeletonJoint joint) {
    static const char *sJointNames[] = {
        "Hip Center",  "Spine",      "Shoulder Center", "Head",           "Left Shoulder",
        "Left Elbow",  "Left Wrist", "Left Hand",       "Right Shoulder", "Right Elbow",
        "Right Wrist", "Right Hand", "Left Hip",        "Left Knee",      "Left Ankle",
        "Right Hip",   "Right Knee", "Right Ankle",     "Left Foot",      "Right Foot"
    };
    MILO_ASSERT_RANGE(joint, 0, DIM(sJointNames), 0x99);
    return sJointNames[joint];
}

const char *CharBoneName(SkeletonJoint joint) {
    static const char *sCharBoneNames[] = {
        "bone_spine1.mesh",     "bone_spine2.mesh",
        "bone_neck.mesh",       "bone_head.mesh",
        "bone_L-upperArm.mesh", "bone_L-foreArm.mesh",
        "bone_L-hand.mesh",     "bone_L-middlefinger03.mesh",
        "bone_R-upperArm.mesh", "bone_R-foreArm.mesh",
        "bone_R-hand.mesh",     "bone_R-middlefinger03.mesh",
        "bone_L-thigh.mesh",    "bone_L-knee.mesh",
        "bone_L-ankle.mesh",    "bone_R-thigh.mesh",
        "bone_R-knee.mesh",     "bone_R-ankle.mesh",
        "bone_L-toe.mesh",      "bone_R-toe.mesh"
    };
    MILO_ASSERT_RANGE(joint, 0, DIM(sCharBoneNames), 0xA0);
    return sCharBoneNames[joint];
}

const char *MirrorBoneName(SkeletonJoint joint) {
    static const char *sMirrorBoneNames[] = {
        "bone_spine1.mesh",     "bone_spine2.mesh",
        "bone_neck.mesh",       "bone_head.mesh",
        "bone_R-upperArm.mesh", "bone_R-foreArm.mesh",
        "bone_R-hand.mesh",     "bone_R-middlefinger03.mesh",
        "bone_L-upperArm.mesh", "bone_L-foreArm.mesh",
        "bone_L-hand.mesh",     "bone_L-middlefinger03.mesh",
        "bone_R-thigh.mesh",    "bone_R-knee.mesh",
        "bone_R-ankle.mesh",    "bone_L-thigh.mesh",
        "bone_L-knee.mesh",     "bone_L-ankle.mesh",
        "bone_R-toe.mesh",      "bone_L-toe.mesh"
    };
    // ghidra also has "camera", "left_arm", "right_arm",  "left_leg", "right_leg",
    // "pelvis"...but not sure why they're there or where they'd be used
    MILO_ASSERT_RANGE(joint, 0, DIM(sMirrorBoneNames), 0xA7);
    return sMirrorBoneNames[joint];
}

int JointParent(SkeletonJoint joint) {
    static int sJointParents[] = { -1, 0,   1, 2,   2,   4, 5,   6,    2,   8,
                                   9,  0xa, 0, 0xc, 0xd, 0, 0xf, 0x10, 0xe, 0x11 };
    MILO_ASSERT_RANGE(joint, 0, DIM(sJointParents), 0xB5);
    return sJointParents[joint];
}

void JointScreenPos(const TrackedJoint &joint, Vector2 &v2) {
    FLOAT fDepthX, fDepthY;
    XMVECTOR vmx;
    vmx.x = joint.unk60.x;
    vmx.y = joint.unk60.y;
    vmx.z = joint.unk60.z;
    NuiTransformSkeletonToDepthImage(vmx, &fDepthX, &fDepthY);
    v2.Set(fDepthX * 0.003125f, fDepthY * 0.004166667f);
}

void JointScreenPos(const TrackedJoint &joint, Vector3 &v3) {
    LONG lDepthX, lDepthY;
    USHORT uDepth;
    XMVECTOR vmx;
    vmx.x = joint.unk60.x;
    vmx.y = joint.unk60.y;
    vmx.z = joint.unk60.z;
    NuiTransformSkeletonToDepthImage(vmx, &lDepthX, &lDepthY, &uDepth);
}
