#include "gesture/HandHeightGestureFilter.h"
#include "BaseSkeleton.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"

HandHeightGestureFilter::HandHeightGestureFilter(SkeletonSide side)
    : mSide(side), mHeightAdjust(0.15), mConfidence(kConfidenceNotTracked) {}

void HandHeightGestureFilter::Clear() { mConfidence = kConfidenceNotTracked; }

void HandHeightGestureFilter::Update(const Skeleton &skeleton, int i2) {
    if (!skeleton.IsTracked()) {
        mConfidence = kConfidenceNotTracked;
        mHandHeight = 0.5;
    } else {
        const TrackedJoint &shoulder = skeleton.ShoulderJoint(mSide);
        const TrackedJoint &hip = skeleton.HipJoint(mSide);
        const TrackedJoint &hand = skeleton.HandJoint(mSide);

        float f2 = shoulder.mPos[kCoordCamera].y - hip.mPos[kCoordCamera].y;
        if (f2 != 0) {
            mHandHeight = (shoulder.mPos[kCoordCamera].y - hand.mPos[kCoordCamera].y) / f2
                + mHeightAdjust;
        } else
            mHandHeight = 0;

        skeleton.HipJoint(mSide);
        skeleton.KneeJoint(mSide);
        skeleton.ElbowJoint(mSide);
        mConfidence = kConfidenceTracked;
    }
}
