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

private:
    void UpdateIsConfident(const TrackedJoint *);
    void UpdateIsSideways(const TrackedJoint *);
    void UpdateIsSitting(const TrackedJoint *);

    float unk4;
    float unk8;
    bool mValid; // 0xc
    float unk10;
    bool unk14;
    bool mSitting; // 0x15
    bool mSideways; // 0x16
    float mSidewaysCutoffThreshold; // 0x18
};
