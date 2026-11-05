#include "gesture/StandingStillGestureFilter.h"
#include "BaseSkeleton.h"
#include "StandingStillGestureFilter.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonQualityFilter.h"
#include "math/Color.h"
#include "math/Mtx.inl"
#include "math/Vec.h"
#include "math/Vec.inl"
#include "obj/Object.h"
#include <cstdlib>

StandingStillGestureFilter::StandingStillGestureFilter()
    : mRequiredMs(500), mForwardFacingCutoff(0.4f), unk4c(false) {
    Clear();
}

StandingStillGestureFilter::~StandingStillGestureFilter() {}

BEGIN_HANDLERS(StandingStillGestureFilter)
    HANDLE_ACTION(clear, Clear())
    HANDLE_ACTION(update, Update(_msg->Int(2), _msg->Int(3)))
    HANDLE_EXPR(check, mSatisfied)
    HANDLE_EXPR(raised_ms, mRaisedMs)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(StandingStillGestureFilter)
    SYNC_PROP(required_ms, mSatisfied)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void StandingStillGestureFilter::SetForwardFacingCutoff(float cutoff) {
    mForwardFacingCutoff = cutoff;
}

void StandingStillGestureFilter::RestoreDefaultForwardFacingCutoff() {
    mForwardFacingCutoff = 0.4f;
}

void StandingStillGestureFilter::Update(int trackingID, int j) {
    const Skeleton *skeleton = TheGestureMgr->GetSkeletonByTrackingID(trackingID);
    if (skeleton) {
        Update(*skeleton, j);
    } else {
        Clear();
    }
}

void StandingStillGestureFilter::Clear() {
    mSatisfied = 0;
    mRaisedMs = 0;
}

void StandingStillGestureFilter::Update(const Skeleton &skeleton, int elapsed_ms) {
    static float sStayDist = 0.5f;
    static float _straightnessRequired = -0.75f;

    int idx = skeleton.SkeletonIndex();
    if (idx < 0 || 6 <= idx) {
        return;
    }

    SkeletonQualityFilter &filter = TheGestureMgr->GetSkeletonQualityFilter(idx);
    filter.Update(skeleton, TheGestureMgr->UIMode());
    bool isTracked = skeleton.IsTracked();
    if (!isTracked && !TheGestureMgr->IsTrackingAllSkeletons()) {
        TheGestureMgr->SetSkeletonStatus(idx, 2);
        mRaisedMs = 0;
    } else {
        if (unk4c) {
        center:
            if (mRaisedMs == 0) {
                mCenter = skeleton.Center();
            }
            bool satisfied = false;

            if (DistanceSquared(mCenter, skeleton.Center()) < sStayDist * sStayDist) {
                mRaisedMs += elapsed_ms;
            } else {
                TheGestureMgr->SetSkeletonStatus(idx, 10);
                mSatisfied = false;
                mRaisedMs = 0;
            }

            if (mRequiredMs > mRaisedMs) {
                satisfied = true;
            } else {
                TheGestureMgr->SetSkeletonStatus(idx, 11);
            }
            mSatisfied = satisfied;
            return;
        }

        if (filter.IsSitting()) {
            TheGestureMgr->SetSkeletonStatus(idx, 3);
        } else {
            if (!filter.IsConfident()) {
                int totalConfidence = filter.GetTotalConfidence();
                TheGestureMgr->SetSkeletonStatus(idx, totalConfidence + 400);
            } else if (filter.IsSideways()) {
                TheGestureMgr->SetSkeletonStatus(idx, 5);
            } else {
                const TrackedJoint &leftShoulderJoint =
                    skeleton.ShoulderJoint(kSkeletonLeft);
                const TrackedJoint &rightShoulderJoint =
                    skeleton.ShoulderJoint(kSkeletonRight);
                Vector3 vShoulders;
                Subtract(
                    rightShoulderJoint.mPos[0], leftShoulderJoint.mPos[0], vShoulders
                );
                Normalize(vShoulders, vShoulders);
                if (fabs(((vShoulders.x + vShoulders.y) * 0) + vShoulders.z)
                    > mForwardFacingCutoff) {
                    TheGestureMgr->SetSkeletonStatus(idx, 6);
                } else {
                    Vector3 vLeftLowerLeg, vLeftUpperLeg;
                    skeleton.ScreenPos(kJointHandLeft, (Vector2 &)vLeftLowerLeg);
                    skeleton.ScreenPos(kJointHandRight, (Vector2 &)vLeftUpperLeg);
                    if (vLeftLowerLeg.x > vLeftUpperLeg.x) {
                        TheGestureMgr->SetSkeletonStatus(idx, 7);
                    } else {
                        Vector3 vRightLowerLeg, vRightUpperLeg;
                        skeleton.ScreenPos(kJointKneeLeft, (Vector2 &)vRightLowerLeg);
                        skeleton.ScreenPos(kJointKneeRight, (Vector2 &)vRightUpperLeg);
                        if (vRightLowerLeg.x > vRightUpperLeg.x) {
                            TheGestureMgr->SetSkeletonStatus(idx, 8);
                        } else {
                            Subtract(
                                skeleton.TrackedJoints()[kJointKneeRight].mPos[0],
                                skeleton.TrackedJoints()[kJointHipRight].mPos[0],
                                vRightUpperLeg
                            );
                            Normalize(vRightUpperLeg, vRightUpperLeg);

                            Subtract(
                                skeleton.TrackedJoints()[kJointKneeRight].mPos[0],
                                skeleton.TrackedJoints()[kJointAnkleRight].mPos[0],
                                vRightLowerLeg
                            );
                            Normalize(vRightLowerLeg, vRightLowerLeg);

                            Subtract(
                                skeleton.TrackedJoints()[kJointKneeLeft].mPos[0],
                                skeleton.TrackedJoints()[kJointHipLeft].mPos[0],
                                vLeftUpperLeg
                            );
                            Normalize(vLeftUpperLeg, vLeftUpperLeg);

                            Subtract(
                                skeleton.TrackedJoints()[kJointKneeLeft].mPos[0],
                                skeleton.TrackedJoints()[kJointAnkleLeft].mPos[0],
                                vLeftLowerLeg
                            );
                            Normalize(vLeftLowerLeg, vLeftLowerLeg);

                            if ((Dot(vRightLowerLeg, vRightUpperLeg)
                                 <= _straightnessRequired)
                                && Dot(vLeftLowerLeg, vLeftUpperLeg)
                                    > _straightnessRequired) {
                                TheGestureMgr->SetSkeletonStatus(idx, 9);
                            } else {
                                goto center;
                            }
                        }
                    }
                }
            }
        }
    }
}
