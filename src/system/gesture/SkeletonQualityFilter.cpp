#include "gesture/SkeletonQualityFilter.h"
#include "BaseSkeleton.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"

SkeletonQualityFilter::SkeletonQualityFilter()
    : unk4(0), unk8(20), mValid(0), unk10(0), mSitting(0), mSideways(0),
      mSidewaysCutoffThreshold(0.55) {}

void SkeletonQualityFilter::Init(float f1, float f2) {
    unk4 = f1;
    unk8 = f2;
}

void SkeletonQualityFilter::SetSidewaysCutoffThreshold(float thresh) {
    mSidewaysCutoffThreshold = thresh;
}

void SkeletonQualityFilter::RestoreDefaultSidewaysCutoffThreshold() {
    mSidewaysCutoffThreshold = 0.55;
}

void SkeletonQualityFilter::UpdateIsConfident(const TrackedJoint *joints) {
    unk10 = 0;
    for (int i = 0; i < kNumJoints; i++) {
        if (joints[i].mJointConf == kConfidenceTracked || i == kJointFootLeft
            || i == kJointFootRight || i == kJointAnkleLeft || i == kJointAnkleRight) {
            unk10 += 1.0f;
        }
    }
    if (unk10 < unk4) {
        unk14 = false;
    }
    if (unk10 > unk8) {
        unk14 = true;
    }
}
