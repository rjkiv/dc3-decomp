#include "gesture/StandingStillGestureFilter.h"
#include "StandingStillGestureFilter.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "math/Vec.h"
#include "obj/Object.h"

StandingStillGestureFilter::StandingStillGestureFilter()
    : mRequiredMs(500), mForwardFacingCutoff(0.4f), unk4c(false) {
    Clear();
}

StandingStillGestureFilter::~StandingStillGestureFilter() {}

BEGIN_HANDLERS(StandingStillGestureFilter)
    HANDLE_ACTION(clear, Clear())
    HANDLE_ACTION(update, Update(_msg->Int(2), _msg->Int(3)))
    HANDLE_EXPR(check, mStandingStill)
    HANDLE_EXPR(raised_ms, mRaisedMs)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(StandingStillGestureFilter)
    SYNC_PROP(required_ms, mStandingStill)
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
    mStandingStill = 0;
    mRaisedMs = 0;
}
