#pragma once
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"
#include "world/PhysicsManager.h"

/** "Physics Volume Trigger, fire events when things enter/exit me" */
class PhysicsVolume : public RndDrawable,
                      public RndTransformable,
                      public RndPollable,
                      public DetectionVolumeListener {
public:
    // Hmx::Object
    virtual ~PhysicsVolume();
    OBJ_CLASSNAME(PhysicsVolume);
    OBJ_SET_TYPE(PhysicsVolume);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndHighlightable
    virtual void Highlight() {}
    // RndDrawable
    virtual bool MakeWorldSphere(Sphere &, bool);
    virtual void DrawShowing();
    // RndPollable
    virtual void Poll();
    virtual void Enter();
    // DetectionVolumeListener
    virtual void OnCollidableEnter(Hmx::Object *, ObjectDir *);
    virtual void OnCollidableExit(Hmx::Object *, ObjectDir *);

    OBJ_MEM_OVERLOAD(0x21)
    NEW_OBJ(PhysicsVolume)

    void SetActiveState(bool);
    void CreatePhysicsVolume(PhysicsManager *);
    void SetCollisionFilter(CollisionFilter);

    static bool sShowing;

protected:
    PhysicsVolume();

    void DestroyPhysicsVolume();
    void ChangeShapeType(int);
    void HalfExtends(Vector3 &);

    DataNode OnSetDirectionalForce(const DataArray *);
    DataNode OnSetDirectionalVelocity(const DataArray *);
    DataNode OnSetTangentialForce(const DataArray *);
    DataNode OnIterateOverlaps(const DataArray *);

    ObjPtr<DetectionVolume> mVolume; // 0x10c
    /** "Volume type generated from PhysicsVolume's transform" */
    int mShapeType; // 0x120
    int mOverlapping; // 0x124
    Vector3 mHalfExtends; // 0x128
    /** "Directional force applied to overlapping physics objects" */
    Vector3 mDirectionalForce; // 0x138
    /** "Flattened planar tangential force applied to overlapping physics objects;
        y is forward/backward, x is outward/inward, z is upward/downward" */
    Vector3 mTangentialForce; // 0x148
    /** "Directional velocity set on overlapping physics objects" */
    Vector3 mDirectionalVelocity; // 0x158
    /** "Radial force applied to overlapping physics objects" */
    float mRadialForce; // 0x168
    /** "What type of collision meshes does this physics volume detect collision with?"
        Options are restricted to the enums that start with "kCollidePhysics". */
    CollisionFilter mCollisionFilter; // 0x16c
    /** "Are we active?" */
    bool mActive; // 0x170
    /** "calls 'while_has_overlaps' handler while
        there are collision objects overlapping the volume" */
    bool mReportOverlaps; // 0x171
};
