#include "gesture/HandHeightGestureFilter.h"
#include "BaseSkeleton.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"

HandHeightGestureFilter::HandHeightGestureFilter(SkeletonSide side)
    : mSide(side), unk8(0.15), unkc(0) {}

void HandHeightGestureFilter::Clear() { unkc = 0; }

void HandHeightGestureFilter::Update(const Skeleton &skeleton, int i2) {
    if (!skeleton.IsTracked()) {
        unkc = 0;
        unk10 = 0.5;
    } else {
        const TrackedJoint &shoulder = skeleton.ShoulderJoint(mSide);
        const TrackedJoint &hip = skeleton.HipJoint(mSide);
        const TrackedJoint &hand = skeleton.HandJoint(mSide);

        float f2 = shoulder.mPos[kCoordCamera].y - hip.mPos[kCoordCamera].y;
        if (f2 != 0) {
            unk10 =
                (shoulder.mPos[kCoordCamera].y - hand.mPos[kCoordCamera].y) / f2
                + unk8;
        } else
            unk10 = 0;

        skeleton.HipJoint(mSide);
        skeleton.KneeJoint(mSide);
        skeleton.ElbowJoint(mSide);
        unkc = 2;
    }
}
