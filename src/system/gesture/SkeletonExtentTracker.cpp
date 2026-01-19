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

SkeletonExtentTracker::SkeletonExtentTracker() : unk3c(-1) {
    SetName("skeleton_extent_tracker", ObjectDir::Main());
}

BEGIN_HANDLERS(SkeletonExtentTracker)
    HANDLE_ACTION(start_tracking, StartTracking(_msg->Int(2)))
    HANDLE_ACTION(stop_tracking, unk3c = -1)
    HANDLE_ACTION(
        apply_to_mesh_verts, ApplyToMeshVerts(_msg->Obj<RndMesh>(2), _msg->Int(3))
    )
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void SkeletonExtentTracker::StartTracking(int i1) {
    unk3c = i1;
    unk34 = FLT_MIN;
    unk38 = FLT_MIN;
    unk2c = FLT_MAX;
    unk30 = FLT_MAX;
}

void SkeletonExtentTracker::Poll() {
    if (unk3c != -1) {
        Skeleton *skeleton = TheGestureMgr->GetSkeletonByTrackingID(unk3c);
        if (skeleton) {
            for (int i = 0; i < kNumJoints; i++) {
                Vector2 pos;
                skeleton->ScreenPos((SkeletonJoint)i, pos);
                unk2c = Min(unk2c, pos.x);
                unk30 = Min(unk30, pos.y - 0.10f);
                unk34 = Max(unk34, pos.x);
                unk38 = Max(unk38, pos.y);
            }
            unk2c = Max(0.0f, unk2c);
            unk30 = Max(0.0f, unk30);
            unk34 = Min(1.0f, unk34);
            unk38 = Min(1.0f, unk38);
        }
    }
}

Hmx::Rect SkeletonExtentTracker::GetViewBox() const {
    Hmx::Rect ret;
    if (unk2c != FLT_MIN && unk2c != FLT_MAX && unk30 != FLT_MIN && unk30 != FLT_MAX) {
        float val = Min(unk38 - unk30, 1.0f);
        ret.Set(((unk34 + unk2c) / 2.0f) - (val / 2.0f), unk38, val, val);
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
