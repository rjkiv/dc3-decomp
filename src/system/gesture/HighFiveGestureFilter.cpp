#include "gesture/HighFiveGestureFilter.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/JointUtl.h"
#include "gesture/Skeleton.h"
#include "math/Vec.h"
#include "obj/Object.h"

HighFiveGestureFilter::HighFiveGestureFilter() : mHighFived(false) {}

HighFiveGestureFilter::~HighFiveGestureFilter() {}

bool HighFiveGestureFilter::CheckHighFive() {
    if (mHighFived) {
        mHighFived = false;
        return true;
    }
    return false;
}

void HighFiveGestureFilter::Update(Skeleton const *skeleton1, Skeleton const *skeleton2) {
    if (skeleton1 && skeleton2) {
        Vector3 vec1, vec2;
        skeleton1->JointPos(kCoordCamera, kJointShoulderCenter, vec1);
        skeleton2->JointPos(kCoordCamera, kJointShoulderCenter, vec2);
        float f = 0.0f;
        for (int i = 0; i < 4; i++) {
            TrackedJoint joint1 = skeleton1->HandJoint((SkeletonSide)i);
            TrackedJoint joint2 = skeleton2->HandJoint((SkeletonSide)i);
        }
    }
}
