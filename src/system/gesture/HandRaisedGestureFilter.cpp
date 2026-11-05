#include "gesture/HandRaisedGestureFilter.h"
#include "BaseSkeleton.h"
#include "Skeleton.h"
#include "StandingStillGestureFilter.h"
#include "gesture/GestureMgr.h"
#include "math/Vec.h"
#include "obj/Object.h"

HandRaisedGestureFilter::HandRaisedGestureFilter()
    : mHandRaised(false), mHandRaisedTimer(0), mNeedHandRaisedTime(500) {
    Clear();
}

HandRaisedGestureFilter::~HandRaisedGestureFilter() {}

BEGIN_HANDLERS(HandRaisedGestureFilter)
    HANDLE_ACTION(clear, Clear())
    HANDLE_ACTION(update, Update(_msg->Int(2), _msg->Int(3)))
    HANDLE_EXPR(check, mHandRaised)
    HANDLE_EXPR(raised_ms, mHandRaisedTimer)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(HandRaisedGestureFilter)
    SYNC_PROP(required_ms, mNeedHandRaisedTime)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void HandRaisedGestureFilter::SetRequiredMs(int ms) {
    mNeedHandRaisedTime = ms;
    mStandingStillFilter.SetRequiredMs(ms / 2);
}

void HandRaisedGestureFilter::SetForwardFacingCutoff(float cutoff) {
    mStandingStillFilter.SetForwardFacingCutoff(cutoff);
}

void HandRaisedGestureFilter::RestoreDefaultForwardFacingCutoff() {
    mStandingStillFilter.RestoreDefaultForwardFacingCutoff();
}

void HandRaisedGestureFilter::Update(int trackingID, int j) {
    const Skeleton *skeleton = TheGestureMgr->GetSkeletonByTrackingID(trackingID);
    if (skeleton) {
        Update(*skeleton, j);
    } else {
        Clear();
    }
}

void HandRaisedGestureFilter::Clear() {
    mHandRaised = false;
    mHandRaisedTimer = 0;
}

void HandRaisedGestureFilter::Update(const Skeleton &skeleton, int elapsed_ms) {
    static float sScreenBorder = 0.05f;
    static float sHandAboveShoulderMin = 0.1f;
    static float sHandShoulderTolerance = 0.3f;

    mHandRaised = false;
    int idx = skeleton.SkeletonIndex();
    TheGestureMgr->SetSkeletonStatus(idx, 1);
    mStandingStillFilter.Update(skeleton, elapsed_ms);
    if (!HandPotentiallyRaised()) {
        mHandRaisedTimer = 0;
    } else {
        if (mHandRaisedTimer == 0) {
            mHandRaisedTimer = 1;
        }
        if (mStandingStillFilter.Satisfied()) {
            bool check = false;
            TheGestureMgr->SetSkeletonStatus(idx, 12);
            for (int i = 0; i < 2; i++) {
                SkeletonSide side = i == 0 ? kSkeletonLeft : kSkeletonRight;
                const TrackedJoint &handJoint = skeleton.HandJoint(side);
                const TrackedJoint &elbowJoint = skeleton.ElbowJoint(side);
                const TrackedJoint &shoulderJoint = skeleton.ShoulderJoint(side);
                Vector2 vHandScreen;
                skeleton.ScreenPos(
                    side == kSkeletonLeft ? kJointHandLeft : kJointHandRight, vHandScreen
                );
                bool withinBorder =
                    vHandScreen.x > sScreenBorder && vHandScreen.x < 1.0f - sScreenBorder;
                if (withinBorder) {
                    if (handJoint.mPos[0].y
                            > shoulderJoint.mPos[0].y + sHandAboveShoulderMin
                        && handJoint.mPos[0].y > elbowJoint.mPos[0].y) {
                        if ((side == kSkeletonRight
                             && handJoint.mPos[0].x
                                 > shoulderJoint.mPos[0].x - sHandShoulderTolerance)
                            || (side == kSkeletonLeft
                                && handJoint.mPos[0].x
                                    < shoulderJoint.mPos[0].x + sHandShoulderTolerance)) {
                            check = true;
                        }
                    }
                }
            }
            if (check) {
                TheGestureMgr->SetSkeletonStatus(idx, 13);
                mHandRaisedTimer += elapsed_ms;
            } else {
                mHandRaisedTimer = 0;
            }
            if (mHandRaisedTimer > mNeedHandRaisedTime) {
                TheGestureMgr->SetSkeletonStatus(idx, 14);
                mHandRaised = true;
            }
        }
    }
}
