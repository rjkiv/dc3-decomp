#include "gesture/SkeletonQualityFilter.h"
#include "BaseSkeleton.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "hamobj/HamGameData.h"
#include "math/Vec.h"
#include "math/Vec.inl"
#include <cstdio>

int dummyunusedskeletonqualityfilterlmao() {
    char buf[50];
    return sprintf_s<50>(buf, "");
}

SkeletonQualityFilter::SkeletonQualityFilter()
    : mMinToLose(0), mNeedToRegain(20), mValid(0), mTotalConfidence(0), mSitting(0),
      mSideways(0), mSidewaysCutoffThreshold(0.55) {}

void SkeletonQualityFilter::Init(float loss, float regain) {
    mMinToLose = loss;
    mNeedToRegain = regain;
}

void SkeletonQualityFilter::SetSidewaysCutoffThreshold(float thresh) {
    mSidewaysCutoffThreshold = thresh;
}

void SkeletonQualityFilter::RestoreDefaultSidewaysCutoffThreshold() {
    mSidewaysCutoffThreshold = 0.55;
}

void SkeletonQualityFilter::UpdateIsConfident(const TrackedJoint *joints) {
    mTotalConfidence = 0.0f;
    for (int i = 0; i < kNumJoints; i++) {
        if (joints[i].mConfidence == kConfidenceTracked || i == kJointFootLeft
            || i == kJointFootRight || i == kJointAnkleLeft || i == kJointAnkleRight) {
            mTotalConfidence += 1.0f;
        }
    }
    if (mTotalConfidence < mMinToLose) {
        mConfident = false;
    }
    if (mTotalConfidence > mNeedToRegain) {
        mConfident = true;
    }
}

void SkeletonQualityFilter::UpdateIsSideways(const TrackedJoint *joints) {
    static float _shouldersBentDot = 0.25f;

    Vector3 vShoulders;
    Subtract(joints[kBoneHandLeft].mPos[0], joints[kBoneHandRight].mPos[0], vShoulders);
    Normalize(vShoulders, vShoulders);

    float threshold = fabsf((vShoulders.x + vShoulders.y) * 0.0f + vShoulders.z);
    float thresh = mSideways ? mSidewaysCutoffThreshold * 0.9f : mSidewaysCutoffThreshold;
    bool side = true;
    if (thresh <= threshold) {
        side = false;
    }
    mSideways = side;

    Vector3 vLeftShoulder;
    Subtract(
        joints[kBoneHandLeft].mPos[0], joints[kBoneArmUpperRight].mPos[0], vLeftShoulder
    );
    Normalize(vLeftShoulder, vLeftShoulder);

    Vector3 vRightShoulder;
    Subtract(
        joints[kBoneHandRight].mPos[0], joints[kBoneArmUpperRight].mPos[0], vRightShoulder
    );
    Normalize(vRightShoulder, vRightShoulder);

    if (Dot(vLeftShoulder, vRightShoulder) > _shouldersBentDot) {
        mSideways = true;
    }
}

void SkeletonQualityFilter::UpdateIsSitting(const TrackedJoint *joint) {
    Vector3 vDiff;
    Subtract(joint[0xD].mPos[0], joint[0xC].mPos[0], vDiff);
    Normalize(vDiff, vDiff);
    Vector3 vDiff2;
    Subtract(joint[0x10].mPos[0], joint[0xF].mPos[0], vDiff2);
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
            mConfident = false;
            return;
        } else if (skeleton.Center() == Vector3::GetZero()) {
            mValid = false;
            mSitting = false;
            mSideways = false;
            mConfident = false;
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
                if (mConfident && !mSideways && !mSitting) {
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
