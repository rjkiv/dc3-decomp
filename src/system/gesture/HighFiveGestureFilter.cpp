#include "gesture/HighFiveGestureFilter.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/JointUtl.h"
#include "gesture/Skeleton.h"
#include "math/Color.h"
#include "math/Vec.h"
#include "obj/Object.h"

HighFiveGestureFilter::HighFiveGestureFilter() : mHighFive(false) {}

HighFiveGestureFilter::~HighFiveGestureFilter() {}

bool HighFiveGestureFilter::CheckHighFive() {
    if (mHighFive) {
        mHighFive = false;
        return true;
    }
    return false;
}

void HighFiveGestureFilter::Update(Skeleton const *skel1, Skeleton const *skel2) {
    static float sHandAboveShoulderDist = -0.05f;
    static float sRequiredDist = 0.15f;
    static float sRequiredDistOffTop = 0.6f;

    if (skel1 && skel2) {
        Vector3 vShoulder1, vShoulder2;
        skel1->JointPos(kCoordCamera, kJointShoulderCenter, vShoulder1);
        skel2->JointPos(kCoordCamera, kJointShoulderCenter, vShoulder2);
        for (int i = 0; i < 4; i++) {
            const TrackedJoint &joint = skel1->HandJoint((SkeletonSide)(i & 1));
            const TrackedJoint &joint_00 = skel2->HandJoint((SkeletonSide)(i >> 1));

            Vector3 jointPos0 = joint.mPos[0];
            Vector3 jointPos1 = joint_00.mPos[0];

            if ((jointPos0.y - sHandAboveShoulderDist > vShoulder1.y)
                || (jointPos1.y - sHandAboveShoulderDist > vShoulder2.y)) {
                if (DistanceSquared(jointPos0, jointPos1)
                    < sRequiredDist * sRequiredDist) {
                    mHighFive = true;
                    return;
                }
            }

            Vector2 hand1Screen, hand2Screen;
            JointScreenPos(joint, hand1Screen);
            JointScreenPos(joint_00, hand2Screen);
            if ((hand1Screen.y < 0) && (hand2Screen.y < 0)) {
                Vector3 vTemp;
                Subtract(jointPos0, jointPos1, vTemp);
                if (LengthSquared(vTemp) < sRequiredDistOffTop * sRequiredDistOffTop) {
                    mHighFive = true;
                    return;
                }
            }
        }
    }
}
