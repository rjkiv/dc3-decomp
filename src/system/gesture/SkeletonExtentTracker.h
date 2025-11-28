#pragma once
#include "math/Geo.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Mesh.h"

class SkeletonExtentTracker : public Hmx::Object {
public:
    // Hmx::Object
    virtual DataNode Handle(DataArray *, bool);

    SkeletonExtentTracker();
    void Poll();
    void ApplyToMeshVerts(RndMesh *, bool) const;

    u32 filler[4];
    int unk3c;

private:
    Hmx::Rect GetViewBox() const;
};
