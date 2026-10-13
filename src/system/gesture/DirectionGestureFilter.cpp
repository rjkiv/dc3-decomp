#include "gesture/DirectionGestureFilter.h"
#include "BaseSkeleton.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonViz.h"
#include "gesture/StandingStillGestureFilter.h"
#include "obj/Task.h"
#include "rndobj/Overlay.h"
#include <cmath>

float DirectionGestureFilter::sLastSwipeTime[6] = { -100, -100, -100, -100, -100, -100 };

#pragma region DirectionGestureFilterSingleUser

DirectionGestureFilterSingleUser::DirectionGestureFilterSingleUser(
    SkeletonSide s1, SkeletonSide s2, float f3, float f4
)
    : mSide(s1), mDirection(s2), mPercentPulled(0), mDisplayedPercentPulled(0),
      mEngaged(0), mAllowAboveShoulder(1), mHighButtonMode(0), mTimeBetweenSwipes(0.5) {
    Clear();
    SkeletonJoint wristJoint, elbowJoint;
    if (s1 == kSkeletonRight) {
        elbowJoint = kJointElbowRight;
        wristJoint = kJointWristRight;
    } else {
        elbowJoint = kJointElbowLeft;
        wristJoint = kJointWristLeft;
    }
    mHandArcDetector.Initialize(s2, wristJoint, elbowJoint, f3);
}

DirectionGestureFilterSingleUser::~DirectionGestureFilterSingleUser() {
    RndOverlay *swipeOverlay = RndOverlay::Find("swipe_direction", false);
    if (swipeOverlay && swipeOverlay->GetCallback() == this) {
        swipeOverlay->SetCallback(nullptr);
    }
}

void DirectionGestureFilterSingleUser::Clear() {
    mConfidence = kConfidenceNotTracked;
    ClearSwipe();
}

void DirectionGestureFilterSingleUser::Update(const Skeleton &skeleton, int elapsedMs) {
    static bool print = false;
    static float sVal = 0.2f;
    mConfidence = kConfidenceTracked;
    if (IsHandValid(skeleton)) {
        mHandArcDetector.Update(skeleton, elapsedMs);
    }
    if (!skeleton.IsTracked()) {
        mHasDirection = false;
        mConfidence = kConfidenceNotTracked;
    } else if (!mHasDirection) {
        mPercentPulled = mHandArcDetector.GetSwipeAmount();
        if (sLastSwipeTime[skeleton.SkeletonIndex()] + mTimeBetweenSwipes
                > TheTaskMgr.UISeconds()
            && sLastSwipeTime[skeleton.SkeletonIndex()] < TheTaskMgr.UISeconds()) {
            ClearSwipe();
        } else if (mPercentPulled >= 1.0f) {
            if (print) {
                mHandArcDetector.PrintJointPath();
            }
            ClearSwipe();
            mHasDirection = true;
            sLastSwipeTime[skeleton.SkeletonIndex()] = TheTaskMgr.UISeconds();
        }
        mDisplayedPercentPulled =
            Clamp(0.0f, 1.0f, (mPercentPulled - sVal) / (1.0f - sVal));
    }
}

void DirectionGestureFilterSingleUser::Draw(const Skeleton &skeleton, SkeletonViz &viz) {
    mHandArcDetector.Draw(skeleton, viz);
    viz.DrawPoint3D(
        skeleton.HandJoint(mSide).mPos[0],
        0.1f,
        IsValidSwipePosition(skeleton) ? Hmx::Color(0, 1, 0) : Hmx::Color(1, 0, 0),
        0.2f
    );
    if (sLastSwipeTime[skeleton.SkeletonIndex()] + mTimeBetweenSwipes
            > TheTaskMgr.UISeconds()
        && sLastSwipeTime[skeleton.SkeletonIndex()] < TheTaskMgr.UISeconds()) {
        viz.DrawPoint3D(
            skeleton.HandJoint(mSide).mPos[0],
            0.1f,
            // FIXME: I need to be able to reuse the color from the first DrawPoint3D call
            IsValidSwipePosition(skeleton) ? Hmx::Color(0, 1, 0) : Hmx::Color(1, 0, 0),
            0.2f
        );
    }
}

static float sValidHandRadius = 0.2f;
static float sValidHandScale = 2.0f;
static float sScrollPosRadius = 0.3f;
static float sScrollPosCenterRadius = 0.3f;

bool DirectionGestureFilterSingleUser::IsHandValid(const Skeleton &skeleton) const {
    return IsValidSwipePosition(skeleton)
        || (mHandArcDetector.NumJointsInPath() > 1U
            && !HandAtSide(skeleton, sValidHandRadius, sValidHandScale, 0.0f));
}

bool DirectionGestureFilterSingleUser::IsValidScrollPos(const Skeleton &skeleton) const {
    if (IsValidSwipePosition(skeleton)) {
        return true;
    } else if (HandAtSide(skeleton, sScrollPosRadius, 1.0f, 0.5f)) {
        return !HandAtSide(skeleton, sScrollPosCenterRadius, 1.0f, 0.0f);
    } else {
        return false;
    }
}

void DirectionGestureFilterSingleUser::ClearSwipe() {
    mPercentPulled = 0;
    mHasDirection = false;
    mDisplayedPercentPulled = 0;
    mHandArcDetector.Clear();
}

bool DirectionGestureFilterSingleUser::IsLockedIn() const {
    return mHandArcDetector.IsLockedIn();
}

void DirectionGestureFilterSingleUser::ResetHoverTimer() {
    mHandArcDetector.ResetHoverTimer();
}

float DirectionGestureFilterSingleUser::UpdateOverlay(RndOverlay *overlay, float f1) {
    return mHandArcDetector.UpdateOverlay(overlay, f1);
}

bool DirectionGestureFilterSingleUser::HandAtSide(
    const Skeleton &skeleton, float f1, float f2, float f3
) const {
    float jointVal =
        skeleton.TrackedJoints()[3].mPos[0].y - skeleton.TrackedJoints()[19].mPos[0].y;

    const TrackedJoint &handJoint = skeleton.HandJoint(mSide);
    const TrackedJoint &hipJoint = skeleton.HipJoint(mSide);
    const TrackedJoint &kneeJoint = skeleton.KneeJoint(mSide);
    const TrackedJoint &elbowJoint = skeleton.ElbowJoint(mSide);
    float jointX = handJoint.mPos[0].x - (f1 * f3 + elbowJoint.mPos[0].x) * f2;
    float jointY = handJoint.mPos[0].y - (kneeJoint.mPos[0].y + hipJoint.mPos[0].y) / 2;
    float jointZ = handJoint.mPos[0].z - (kneeJoint.mPos[0].z + hipJoint.mPos[0].z) / 2;

    return sqrt(jointX * jointX + jointZ * jointZ + jointY * jointY)
        <= jointVal * 0.59171593f * f1;
}

#pragma endregion
#pragma region DirectionGestureFilterDoubleUser

DirectionGestureFilterDoubleUser::DirectionGestureFilterDoubleUser(
    SkeletonSide s1, SkeletonSide s2, float f3, float f4
)
    : mFilter1(new DirectionGestureFilterSingleUser(s1, s2, f3, f4)),
      mFilter2(new DirectionGestureFilterSingleUser(s1, s2, f3, f4)) {
    for (int i = 0; i < 2; i++) {
        mStandingStillFilters[i] = new StandingStillGestureFilter();
        mStandingStillFilters[i]->SetRequiredMs(750);
        mStandingStillFilters[i]->SetUnk4c(true);
    }
}

DirectionGestureFilterDoubleUser::~DirectionGestureFilterDoubleUser() {
    delete mFilter1;
    delete mFilter2;
    for (int i = 0; i < 2; i++) {
        delete mStandingStillFilters[i];
    }
}

void DirectionGestureFilterDoubleUser::Clear() {
    mFilter1->Clear();
    mFilter2->Clear();
}

JointConfidence DirectionGestureFilterDoubleUser::Confidence() const {
    return Max(mFilter1->Confidence(), mFilter2->Confidence());
}

void DirectionGestureFilterDoubleUser::Update(const Skeleton &skeleton, int ms) {
    for (int i = 0; i < 2; i++) {
        mStandingStillFilters[i]->Update(TheGestureMgr->GetPlayerSkeletonID(i), ms);
    }
    int i1, i2;
    GetValidSkeletons(i1, i2);
    mFilter1->Update(TheGestureMgr->GetSkeleton(i1), ms);
    mFilter2->Update(TheGestureMgr->GetSkeleton(i2), ms);
}

void DirectionGestureFilterDoubleUser::Draw(const Skeleton &skeleton, SkeletonViz &viz) {
    mFilter1->Draw(skeleton, viz);
}

bool DirectionGestureFilterDoubleUser::HasDirection() const {
    return mFilter1->HasDirection() || mFilter2->HasDirection();
}

float DirectionGestureFilterDoubleUser::GetPercentPulled() const {
    return Max(mFilter1->GetPercentPulled(), mFilter2->GetPercentPulled());
}

void DirectionGestureFilterDoubleUser::ClearSwipe() {
    mFilter1->ClearSwipe();
    mFilter2->ClearSwipe();
}

bool DirectionGestureFilterDoubleUser::IsLockedIn() const {
    return mFilter1->IsLockedIn() || mFilter2->IsLockedIn();
}

void DirectionGestureFilterDoubleUser::SetEngaged(bool engaged) {
    mFilter1->SetEngaged(engaged);
    mFilter2->SetEngaged(engaged);
}

void DirectionGestureFilterDoubleUser::ResetHoverTimer() {
    mFilter1->ResetHoverTimer();
    mFilter2->ResetHoverTimer();
}

void DirectionGestureFilterDoubleUser::SetAllowAboveShoulder(bool allow) {
    mFilter1->SetAllowAboveShoulder(allow);
    mFilter2->SetAllowAboveShoulder(allow);
}

void DirectionGestureFilterDoubleUser::SetHighButtonMode(bool set) {
    mFilter1->SetHighButtonMode(set);
    mFilter2->SetHighButtonMode(set);
}

bool DirectionGestureFilterDoubleUser::IsValidScrollPos(const Skeleton &skel) const {
    int p1, p2;
    GetValidSkeletons(p1, p2);
    return p1 >= 0 && mFilter1->IsValidScrollPos(TheGestureMgr->GetSkeleton(p1))
        || p2 >= 0 && mFilter2->IsValidScrollPos(TheGestureMgr->GetSkeleton(p2));
}

bool DirectionGestureFilterDoubleUser::IsHandValid(const Skeleton &skel) const {
    int p1, p2;
    GetValidSkeletons(p1, p2);
    return (p1 >= 0 && mFilter1->IsHandValid(TheGestureMgr->GetSkeleton(p1))
            && mStandingStillFilters[0]->Satisfied())
        || (p2 >= 0 && mFilter2->IsHandValid(TheGestureMgr->GetSkeleton(p2))
            && mStandingStillFilters[1]->Satisfied());
}

void DirectionGestureFilterDoubleUser::GetValidSkeletons(int &p1, int &p2) const {
    int id1 = TheGestureMgr->GetPlayerSkeletonID(0);
    int id2 = TheGestureMgr->GetPlayerSkeletonID(1);

    p1 = (id1 != -1) ? TheGestureMgr->GetSkeletonIndexByTrackingID(id1) : -1;
    p2 = (id2 != -1) ? TheGestureMgr->GetSkeletonIndexByTrackingID(id2) : -1;

    if (p1 != -1 && !TheGestureMgr->IsSkeletonValid(p1)) {
        p1 = -1;
    }
    if (p2 != -1 && !TheGestureMgr->IsSkeletonValid(p2)) {
        p2 = -1;
    }
}
