#pragma once
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"

class HandHeightGestureFilter {
public:
    HandHeightGestureFilter(SkeletonSide);
    virtual ~HandHeightGestureFilter() {}

    void Update(const Skeleton &, int);
    void Clear();

    float GetHeightAdjust() const { return mHeightAdjust; }
    JointConfidence Confidence() const { return mConfidence; }
    float GetHandHeight() { return mHandHeight; }
    void SetHeightAdjust(float newHeightAdjust) { mHeightAdjust = newHeightAdjust; }

protected:
    SkeletonSide mSide; // 0x4
    float mHeightAdjust; // 0x8
    JointConfidence mConfidence; // 0xC
    float mHandHeight; // 0x10
};
