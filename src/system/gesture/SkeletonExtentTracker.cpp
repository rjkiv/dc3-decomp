#include "gesture/SkeletonExtentTracker.h"
#include "math/Geo.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Mesh.h"

SkeletonExtentTracker::SkeletonExtentTracker() : unk3c(-1) {
    SetName("skeleton_extent_tracker", ObjectDir::Main());
}

BEGIN_HANDLERS(SkeletonExtentTracker)
    HANDLE_ACTION(
        apply_to_mesh_verts, ApplyToMeshVerts(_msg->Obj<RndMesh>(2), !(_msg->Int(3) == 0))
    )
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
