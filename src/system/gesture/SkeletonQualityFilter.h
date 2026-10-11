#pragma once
#include "gesture/Skeleton.h"

class SkeletonQualityFilter {
public:
    SkeletonQualityFilter();
    virtual ~SkeletonQualityFilter() {}

    void Init(float minToLose, float needToRegain);
    void SetSidewaysCutoffThreshold(float thresh);
    void RestoreDefaultSidewaysCutoffThreshold();
    void Update(const Skeleton &skeleton, bool uiMode);
    bool IsSkeletonValid() const { return mValid; }
    bool IsSitting() const { return mSitting; }
    bool IsSideways() const { return mSideways; }
    bool IsConfident() const { return mConfident; }
    float GetTotalConfidence() const { return mTotalConfidence; }

private:
    void UpdateIsConfident(const TrackedJoint *joints);
    void UpdateIsSideways(const TrackedJoint *joints);
    void UpdateIsSitting(const TrackedJoint *joints);

    float mMinToLose; // 0x4
    float mNeedToRegain; // 0x8
    bool mValid; // 0xc
    float mTotalConfidence; // 0x10
    bool mConfident; // 0x14
    bool mSitting; // 0x15
    bool mSideways; // 0x16
    float mSidewaysCutoffThreshold; // 0x18
};
