#include "gesture/SkeletonQualityFilter.h"
#include "BaseSkeleton.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "hamobj/HamGameData.h"
#include "math/Vec.h"

SkeletonQualityFilter::SkeletonQualityFilter()
    : mConfidenceLossThreshold(0), mConfidenceRegainThreshold(20), mValid(0),
      mConfidence(0), mSitting(0), mSideways(0), mSidewaysCutoffThreshold(0.55) {}

void SkeletonQualityFilter::Init(float loss, float regain) {
    mConfidenceLossThreshold = loss;
    mConfidenceRegainThreshold = regain;
}

void SkeletonQualityFilter::SetSidewaysCutoffThreshold(float thresh) {
    mSidewaysCutoffThreshold = thresh;
}

void SkeletonQualityFilter::RestoreDefaultSidewaysCutoffThreshold() {
    mSidewaysCutoffThreshold = 0.55;
}

void SkeletonQualityFilter::UpdateIsConfident(const TrackedJoint *joints) {
    mConfidence = 0.0f;
    for (int i = 0; i < kNumJoints; i++) {
        if (joints[i].mJointConf == kConfidenceTracked || i == kJointFootLeft
            || i == kJointFootRight || i == kJointAnkleLeft || i == kJointAnkleRight) {
            mConfidence += 1.0f;
        }
    }
    if (mConfidence < mConfidenceLossThreshold) {
        mIsConfident = false;
    }
    if (mConfidence > mConfidenceRegainThreshold) {
        mIsConfident = true;
    }
}

void SkeletonQualityFilter::UpdateIsSideways(const TrackedJoint *joint) {
    Vector3 vDiff;
    Subtract(joint[8].mJointPos[0], joint[4].mJointPos[0], vDiff);
    Normalize(vDiff, vDiff);
    float threshold = fabsf((vDiff.x + vDiff.y) * 0.0f + vDiff.z);
    float thresh = mSideways ? mSidewaysCutoffThreshold * 0.9f : mSidewaysCutoffThreshold;
    bool side = true;
    if (thresh <= threshold) {
        side = false;
    }
    mSideways = side;
    Subtract(joint[8].mJointPos[0], joint[2].mJointPos[0], vDiff);
    Normalize(vDiff, vDiff);
    Vector3 vDiff2;
    Subtract(joint[4].mJointPos[0], joint[2].mJointPos[0], vDiff2);
    Normalize(vDiff2, vDiff2);
    if (0.25f < Dot(vDiff, vDiff2)) {
        mSideways = true;
    }
}

void SkeletonQualityFilter::UpdateIsSitting(const TrackedJoint *joint) {
    Vector3 vDiff;
    Subtract(joint[0xD].mJointPos[0], joint[0xC].mJointPos[0], vDiff);
    Normalize(vDiff, vDiff);
    Vector3 vDiff2;
    Subtract(joint[0x10].mJointPos[0], joint[0xF].mJointPos[0], vDiff2);
    Normalize(vDiff2, vDiff2);
    if (!mSitting) {
        if (vDiff.y > -0.7f && vDiff2.y > -0.7f) {
            mSitting = true;
        }
    } else if (vDiff.y < -0.8f && vDiff2.y < -0.8f) {
        mSitting = false;
    }
}

void SkeletonQualityFilter::Update(const Skeleton &skeleton, bool b2) {
    if (skeleton.SkeletonIndex() >= 0 && skeleton.SkeletonIndex() < 6) {
        if (!skeleton.IsTracked() && !TheGestureMgr->IsTrackingAllSkeletons()) {
            mValid = false;
            mSitting = false;
            mSideways = false;
            mIsConfident = false;
            return;
        } else if (skeleton.GetUnkab0() == Vector3::GetZero()) {
            mValid = false;
            mSitting = false;
            mSideways = false;
            mIsConfident = false;
            return;
        } else {
            bool b3 = false;
            for (int i = 0; i < 2; i++) {
                if (TheGameData->Player(i)->GetSkeletonTrackingID()
                    == skeleton.TrackingID()) {
                    b3 = TheGameData->Player(i)->IsPlaying();
                    break;
                }
            }
            const TrackedJoint *joints = skeleton.TrackedJoints();
            UpdateIsConfident(joints);
            UpdateIsSideways(joints);
            UpdateIsSitting(joints);
            if (b2 || !b3) {
                if (mIsConfident && !mSideways && !mSitting) {
                    b3 = true;
                } else {
                    b3 = false;
                }
            } else {
                b3 = true;
            }
            mValid = b3;
            return;
        }
    }
}
