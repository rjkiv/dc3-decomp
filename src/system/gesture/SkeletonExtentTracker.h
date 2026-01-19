#pragma once
#include "math/Geo.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Mesh.h"

class SkeletonExtentTracker : public Hmx::Object {
public:
    SkeletonExtentTracker();
    // Hmx::Object
    virtual DataNode Handle(DataArray *, bool);

    void Poll();
    void ApplyToMeshVerts(RndMesh *, bool) const;
    void StartTracking(int);

private:
    Hmx::Rect GetViewBox() const;

    float unk2c;
    float unk30;
    float unk34;
    float unk38;
    int unk3c;
};
