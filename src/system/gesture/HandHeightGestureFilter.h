#pragma once
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"

class HandHeightGestureFilter {
public:
    HandHeightGestureFilter(SkeletonSide);
    virtual ~HandHeightGestureFilter() {}

    void Update(const Skeleton &, int);
    void Clear();

    float GetUnk8() const { return unk8; }
    int GetUnkC() const { return unkc; }
    float GetUnk10() const { return unk10; }

protected:
    SkeletonSide mSide; // 0x4
    float unk8; // 0x8
    int unkc; // 0xc - height?
    float unk10; // 0x10
};
