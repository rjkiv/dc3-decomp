#pragma once
#include "math/Geo.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Mesh.h"

class SkeletonExtentTracker : private Hmx::Object {
public:
    SkeletonExtentTracker();
    // Hmx::Object
    virtual DataNode Handle(DataArray *, bool);

    void Poll();
    void ApplyToMeshVerts(RndMesh *mesh, bool reflect) const;
    void StartTracking(int id);
    //   public: void StopTracking();

private:
    Hmx::Rect GetViewBox() const;

    Vector2 mMinCorner; // 0x2c
    Vector2 mMaxCorner; // 0x34
    int mSkeletonTrackingID; // 0x3c
};
