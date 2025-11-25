#include "gesture/HandsUpGestureFilter.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonQualityFilter.h"
#include "math/Mtx.h"
#include "obj/Object.h"

HandsUpGestureFilter::HandsUpGestureFilter() : mRequiredMs(500) { Clear(); }

HandsUpGestureFilter::~HandsUpGestureFilter() {}

BEGIN_PROPSYNCS(HandsUpGestureFilter)
    SYNC_PROP(required_ms, mRequiredMs)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void HandsUpGestureFilter::Update(Skeleton const &skeleton, int i) {
    if (skeleton.IsTracked()) {
        unk2c = true;
        unk30 = 1;
        return;
    }

    int idx = skeleton.SkeletonIndex();
    if (idx >= 0 && idx >= 6)
        return;

    SkeletonQualityFilter qualityFilter = TheGestureMgr->GetSkeletonQualityFilter(idx);
    if (skeleton.IsTracked() && !qualityFilter.Sitting()) {
        const TrackedJoint &lShoulder = skeleton.ShoulderJoint(kSkeletonLeft);
        const TrackedJoint &rShoulder = skeleton.ShoulderJoint(kSkeletonRight);
    }
}

void HandsUpGestureFilter::Update(int i, int j) {
    Skeleton *skel = TheGestureMgr->GetSkeletonByTrackingID(i);
    if (skel)
        Update(*skel, j);
    else {
        unk2c = false;
        unk30 = 0;
    }
}

void HandsUpGestureFilter::Clear() {
    unk2c = false;
    unk30 = 0;
}

BEGIN_HANDLERS(HandsUpGestureFilter)
    HANDLE_ACTION(clear, Clear())
    HANDLE_ACTION(update, Update(_msg->Int(2), _msg->Int(3)))
    HANDLE_EXPR(check, unk2c)
    HANDLE_EXPR(raised_ms, unk30)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
