#include "gesture/StandingStillGestureFilter.h"
#include "StandingStillGestureFilter.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "obj/Object.h"

StandingStillGestureFilter::StandingStillGestureFilter()
    : unk34(500), mForwardFacingCutoff(0x3ecccccd), unk4c(false) {
    Clear();
}

StandingStillGestureFilter::~StandingStillGestureFilter() {}

BEGIN_PROPSYNCS(StandingStillGestureFilter)
    SYNC_PROP(required_ms, mHasRequiredMs)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void StandingStillGestureFilter::SetForwardFacingCutoff(float f) {
    mForwardFacingCutoff = f;
}

void StandingStillGestureFilter::RestoreDefaultForwardFacingCutoff() {
    mForwardFacingCutoff = 0x3ecccccd;
}

void StandingStillGestureFilter::Update(int i, int j) {
    const Skeleton *skeleton = TheGestureMgr->GetSkeletonByTrackingID(i);
    if (skeleton) {
        Update(*skeleton, j);
    } else {
        Clear();
    }
}

void StandingStillGestureFilter::Clear() {
    mHasRequiredMs = 0;
    mRaisedMs = 0;
}

BEGIN_HANDLERS(StandingStillGestureFilter)
    HANDLE_ACTION(clear, Clear())
    HANDLE_ACTION(update, Update(_msg->Int(2), _msg->Int(3)))
    HANDLE_EXPR(check, mHasRequiredMs)
    HANDLE_EXPR(raised_ms, mRaisedMs)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
