#pragma once
#include "gesture/Skeleton.h"

class SkeletonQualityFilter {
public:
    SkeletonQualityFilter();
    virtual ~SkeletonQualityFilter() {}

    void Init(float, float);
    void SetSidewaysCutoffThreshold(float);
    void RestoreDefaultSidewaysCutoffThreshold();
    void Update(const Skeleton &, bool);
    bool Valid() const { return mValid; }
    bool Sitting() const { return mSitting; }
    bool Sideways() const { return mSideways; }
    bool IsConfident() const { return mIsConfident; }
    float Confidence() const { return mConfidence; }

private:
    void UpdateIsConfident(const TrackedJoint *);
    void UpdateIsSideways(const TrackedJoint *);
    void UpdateIsSitting(const TrackedJoint *);

    float mConfidenceLossThreshold; // 0x4
    float mConfidenceRegainThreshold; // 0x8
    bool mValid; // 0xc
    float mConfidence; // 0x10
    bool mIsConfident; // 0x14
    bool mSitting; // 0x15
    bool mSideways; // 0x16
    float mSidewaysCutoffThreshold; // 0x18
};
