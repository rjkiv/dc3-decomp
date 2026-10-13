#include "gesture/SkeletonExtentTracker.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "math/Geo.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Mesh.h"
#include <float.h>

SkeletonExtentTracker::SkeletonExtentTracker() : mSkeletonTrackingID(-1) {
    SetName("skeleton_extent_tracker", ObjectDir::Main());
}

BEGIN_HANDLERS(SkeletonExtentTracker)
    HANDLE_ACTION(start_tracking, StartTracking(_msg->Int(2)))
    HANDLE_ACTION(stop_tracking, mSkeletonTrackingID = -1)
    HANDLE_ACTION(
        apply_to_mesh_verts, ApplyToMeshVerts(_msg->Obj<RndMesh>(2), _msg->Int(3))
    )
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void SkeletonExtentTracker::StartTracking(int i1) {
    mSkeletonTrackingID = i1;
    mMinCorner = Vector2(FLT_MAX, FLT_MAX);
    mMaxCorner = Vector2(FLT_MIN, FLT_MIN);
}

void SkeletonExtentTracker::Poll() {
    if (mSkeletonTrackingID != -1) {
        Skeleton *skeleton = TheGestureMgr->GetSkeletonByTrackingID(mSkeletonTrackingID);
        if (skeleton) {
            for (int i = 0; i < kNumJoints; i++) {
                Vector2 pos;
                skeleton->ScreenPos((SkeletonJoint)i, pos);
                mMinCorner.x = Min(mMinCorner.x, pos.x);
                mMinCorner.y = Min(mMinCorner.y, pos.y - 0.10f);
                mMaxCorner.x = Max(mMaxCorner.x, pos.x);
                mMaxCorner.y = Max(mMaxCorner.y, pos.y);
            }
            mMinCorner.x = Max(0.0f, mMinCorner.x);
            mMinCorner.y = Max(0.0f, mMinCorner.y);
            mMaxCorner.x = Min(1.0f, mMaxCorner.x);
            mMaxCorner.y = Min(1.0f, mMaxCorner.y);
        }
    }
}

Hmx::Rect SkeletonExtentTracker::GetViewBox() const {
    Hmx::Rect ret;
    if (mMinCorner.x != FLT_MIN && mMinCorner.x != FLT_MAX && mMinCorner.y != FLT_MIN
        && mMinCorner.y != FLT_MAX) {
        float val = Min(mMaxCorner.y - mMinCorner.y, 1.0f);
        ret.Set(((mMaxCorner.x + mMinCorner.x) / 2) - (val / 2), mMinCorner.y, val, val);
    } else {
        ret.Set(0, 0, 1, 1);
    }
    return ret;
}

void SkeletonExtentTracker::ApplyToMeshVerts(RndMesh *mesh, bool b2) const {
    Hmx::Rect box = GetViewBox();
    MILO_ASSERT(mesh->Verts().size() == 16, 0x43);
    for (int i = 0; i < 4; i++) {
    }
}
