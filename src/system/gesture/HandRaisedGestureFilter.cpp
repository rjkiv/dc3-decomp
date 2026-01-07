#include "gesture/HandRaisedGestureFilter.h"
#include "StandingStillGestureFilter.h"
#include "gesture/GestureMgr.h"
#include "obj/Object.h"

HandRaisedGestureFilter::HandRaisedGestureFilter()
    : mHandRaised(false), mRaisedMs(0), mRequiredMs(500) {
    Clear();
}

HandRaisedGestureFilter::~HandRaisedGestureFilter() {}

BEGIN_HANDLERS(HandRaisedGestureFilter)
    HANDLE_ACTION(clear, Clear())
    HANDLE_ACTION(update, Update(_msg->Int(2), _msg->Int(3)))
    HANDLE_EXPR(check, mHandRaised)
    HANDLE_EXPR(raised_ms, mRaisedMs)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(HandRaisedGestureFilter)
    SYNC_PROP(required_ms, mRequiredMs)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void HandRaisedGestureFilter::SetRequiredMs(int ms) {
    mRequiredMs = ms;
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
    mRaisedMs = 0;
}
