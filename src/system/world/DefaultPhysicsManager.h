#pragma once
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Mesh.h"
#include "world/PhysicsManager.h"

class DefaultPhysicsManager : public PhysicsManager {
public:
    DefaultPhysicsManager(RndDir *);
    virtual ~DefaultPhysicsManager() {}
    virtual bool Replace(ObjRef *, Hmx::Object *);
    virtual void Poll();
    virtual RayCastContainer *MakeContainer(const Box &, unsigned int);
    virtual DetectionVolume *MakeDetectionVolume(
        DetectionVolumeListener *, const Transform &, PhysicsVolumeType, CollisionFilter
    );
    virtual void CastRays(RayCast *, int);
    virtual void CastRays(const Segment *, RayCastListener *, int, unsigned int);

protected:
    virtual void ActivateCollidable(Hmx::Object *);
    virtual void DeactivateCollidable(Hmx::Object *);
    virtual void RemoveAll();
    virtual void AddCollidable(Hmx::Object *, ObjectDir *, bool);
    virtual void RemoveCollidable(Hmx::Object *);

    ObjPtrList<Hmx::Object> unk40; // 0x40
    std::map<Hmx::Object *, ObjectDir *> unk54; // 0x54 - mCollidableMap?
    std::list<RndMesh *> mActiveCollidables; // 0x6c
    std::list<RndMesh *> mInactiveCollidables; // 0x74
};

class RayCastDefaultContainer : public RayCastContainer {
public:
    RayCastDefaultContainer(
        const Box &, std::list<RndMesh *>, std::map<Hmx::Object *, ObjectDir *> &
    );
    virtual ~RayCastDefaultContainer() {}
    virtual Hmx::Object *FindNearest(const Segment &, float &, Vector3 &, Hmx::Object *&);
    virtual void SetFilter(int);

protected:
    std::list<std::pair<RndMesh *, ObjectDir *> > unk4; // 0x4
};

class DefaultDetectionVolume : public DetectionVolume {
public:
    DefaultDetectionVolume(DetectionVolumeListener *);
    virtual ~DefaultDetectionVolume() {}
    virtual void SetActiveState(bool active) { mActiveState = active; }
    virtual bool GetActiveState() const { return mActiveState; }
    virtual void Reset(const Transform &) {}
    virtual void ApplyRadialForce(float) {}
    virtual void Release() { delete this; }

protected:
    DetectionVolumeListener *mListener; // 0x2c
    bool mActiveState; // 0x30
};
