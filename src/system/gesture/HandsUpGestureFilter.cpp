#include "gesture/HandsUpGestureFilter.h"
#include "BaseSkeleton.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonQualityFilter.h"
#include "math/Color.h"
#include "math/Mtx.h"
#include "math/Mtx.inl"
#include "math/Vec.h"
#include "obj/Object.h"
#include <cmath>

HandsUpGestureFilter::HandsUpGestureFilter() : mNeedHandsUpTime(500) { Clear(); }

HandsUpGestureFilter::~HandsUpGestureFilter() {}

BEGIN_PROPSYNCS(HandsUpGestureFilter)
    SYNC_PROP(required_ms, mNeedHandsUpTime)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void HandsUpGestureFilter::Update(Skeleton const &skeleton, int elapsed_ms) {
    static float sHandShoulderTolerance = 0.0f;
    static bool _fakeHandsUp = false;
    static float sHandAboveShoulderMin = 0.1f;
    static float sScreenBorder = 0.05f;

    if (_fakeHandsUp) {
        mHandsUp = true;
        mHandsUpTimer = 1;
        return;
    }

    int idx = skeleton.SkeletonIndex();
    if (idx < 0 || 6 <= idx) {
        return;
    }

    SkeletonQualityFilter &qualityFilter = TheGestureMgr->GetSkeletonQualityFilter(idx);
    if (skeleton.IsTracked() && !qualityFilter.IsSitting()
        && qualityFilter.IsConfident()) {
        const TrackedJoint &leftShoulder = skeleton.ShoulderJoint(kSkeletonLeft);
        const TrackedJoint &rightShoulder = skeleton.ShoulderJoint(kSkeletonRight);
        Vector3 vShoulders;
        Subtract(rightShoulder.mPos[0], leftShoulder.mPos[0], vShoulders);
        Normalize(vShoulders, vShoulders);

        if (fabs((vShoulders.x + vShoulders.y) * 0 + vShoulders.z) <= 0.8f) {
            const TrackedJoint &rightHand = skeleton.HandJoint(kSkeletonRight);
            const TrackedJoint &rightElbow = skeleton.ElbowJoint(kSkeletonRight);
            const TrackedJoint &rightShoulder = skeleton.ShoulderJoint(kSkeletonRight);
            const TrackedJoint &leftHand = skeleton.HandJoint(kSkeletonLeft);
            const TrackedJoint &leftElbow = skeleton.ElbowJoint(kSkeletonLeft);
            const TrackedJoint &leftShoulder = skeleton.ShoulderJoint(kSkeletonLeft);
            Vector2 vRightHandScreen, vLeftHandScreen;
            skeleton.ScreenPos(kJointHandRight, vRightHandScreen);
            skeleton.ScreenPos(kJointHandLeft, vLeftHandScreen);

            bool rightWithinBorder = vRightHandScreen.x > sScreenBorder
                && vRightHandScreen.x < 1.0f - sScreenBorder;
            bool leftWithinBorder = vLeftHandScreen.x > sScreenBorder
                && vLeftHandScreen.x < 1.0f - sScreenBorder;

            if ((rightWithinBorder && leftWithinBorder)
                && (rightHand.mPos[0].y > rightShoulder.mPos[0].y + sHandAboveShoulderMin
                    && rightHand.mPos[0].y > rightElbow.mPos[0].y)
                && (rightHand.mPos[0].x > rightShoulder.mPos[0].x - sHandShoulderTolerance
                    || leftHand.mPos[0].x
                        < leftShoulder.mPos[0].x + sHandShoulderTolerance)
                && (leftHand.mPos[0].y > leftShoulder.mPos[0].y + sHandAboveShoulderMin)
                && (leftHand.mPos[0].y > leftElbow.mPos[0].y)) {
                mHandsUpTimer += elapsed_ms;
            } else {
                mHandsUpTimer = 0;
            }

            if (mHandsUpTimer > mNeedHandsUpTime) {
                mHandsUp = true;
            } else {
                mHandsUp = false;
            }
            return;
        }
    }
    Clear();
}

void HandsUpGestureFilter::Update(int trackingId, int elapsed_ms) {
    Skeleton *skel = TheGestureMgr->GetSkeletonByTrackingID(trackingId);
    if (skel)
        Update(*skel, elapsed_ms);
    else {
        Clear();
    }
}

void HandsUpGestureFilter::Clear() {
    mHandsUp = false;
    mHandsUpTimer = 0;
}

BEGIN_HANDLERS(HandsUpGestureFilter)
    HANDLE_ACTION(clear, Clear())
    HANDLE_ACTION(update, Update(_msg->Int(2), _msg->Int(3)))
    HANDLE_EXPR(check, mHandsUp)
    HANDLE_EXPR(raised_ms, mHandsUpTimer)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
