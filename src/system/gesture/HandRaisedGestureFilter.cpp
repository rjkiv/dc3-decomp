#include "gesture/HandRaisedGestureFilter.h"
#include "StandingStillGestureFilter.h"
#include "gesture/GestureMgr.h"
#include "obj/Object.h"

HandRaisedGestureFilter::HandRaisedGestureFilter()
    : unk2c(false), unk30(0), mRequiredMs(500) {
    Clear();
}

HandRaisedGestureFilter::~HandRaisedGestureFilter() {}

BEGIN_PROPSYNCS(HandRaisedGestureFilter)
    SYNC_PROP(required_ms, mRequiredMs)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void HandRaisedGestureFilter::SetRequiredMs(int i) {
    mRequiredMs = i;
    unk38.unk34 = i >> 1;
}

void HandRaisedGestureFilter::SetForwardFacingCutoff(float f) {
    unk38.SetForwardFacingCutoff(f);
}

void HandRaisedGestureFilter::RestoreDefaultForwardFacingCutoff() {
    unk38.RestoreDefaultForwardFacingCutoff();
}

void HandRaisedGestureFilter::Update(int i, int j) {
    const Skeleton *skeleton = TheGestureMgr->GetSkeletonByTrackingID(i);
    if (skeleton) {
        Update(*skeleton, j);
    } else {
        Clear();
    }
}

void HandRaisedGestureFilter::Clear() {
    unk2c = false;
    unk30 = 0;
}

BEGIN_HANDLERS(HandRaisedGestureFilter)
    HANDLE_ACTION(clear, Clear())
    HANDLE_ACTION(update, Update(_msg->Int(2), _msg->Int(3)))
    HANDLE_EXPR(check, unk2c)
    HANDLE_EXPR(raised_ms, unk30)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
