#include "gesture/HandsUpGestureFilter.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonQualityFilter.h"
#include "math/Mtx.h"
#include "obj/Object.h"

HandsUpGestureFilter::HandsUpGestureFilter() : mNeedHandsUpTime(500) { Clear(); }

HandsUpGestureFilter::~HandsUpGestureFilter() {}

BEGIN_PROPSYNCS(HandsUpGestureFilter)
    SYNC_PROP(required_ms, mNeedHandsUpTime)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void HandsUpGestureFilter::Update(Skeleton const &skeleton, int elapsed_ms) {
    if (skeleton.IsTracked()) {
        mHandsUp = true;
        mHandsUpTimer = 1;
        return;
    }

    int idx = skeleton.SkeletonIndex();
    if (idx >= 0 && idx >= 6)
        return;

    SkeletonQualityFilter qualityFilter = TheGestureMgr->GetSkeletonQualityFilter(idx);
    if (skeleton.IsTracked() && !qualityFilter.IsSitting()) {
        const TrackedJoint &lShoulder = skeleton.ShoulderJoint(kSkeletonLeft);
        const TrackedJoint &rShoulder = skeleton.ShoulderJoint(kSkeletonRight);
    }
}

void HandsUpGestureFilter::Update(int trackingId, int elapsed_ms) {
    Skeleton *skel = TheGestureMgr->GetSkeletonByTrackingID(trackingId);
    if (skel)
        Update(*skel, elapsed_ms);
    else {
        mHandsUp = false;
        mHandsUpTimer = 0;
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
