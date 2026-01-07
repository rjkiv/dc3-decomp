#pragma once
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"

class HandHeightGestureFilter {
public:
    HandHeightGestureFilter(SkeletonSide);
    virtual ~HandHeightGestureFilter() {}

    void Update(const Skeleton &, int);
    void Clear();

protected:
    SkeletonSide mSide; // 0x4
    float unk8; // 0x8
    int unkc; // 0xc - height?
    float unk10; // 0x10
};
