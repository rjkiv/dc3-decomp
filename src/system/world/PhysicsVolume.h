#pragma once
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

enum PhysicsVolumeType {
    kPhysicsVolumeBox = 0,
    kPhysicsVolumeSphere = 1
};

enum CollisionFilter {
    COL_FILTER_ALL = 0,
    COL_FILTER_FIXED = 1,
    COL_FILTER_FIXED_DETECTABLE = 2,
    COL_FILTER_KEYFRAMED = 3,
    COL_FILTER_DYNAMIC = 4,
    COL_FILTER_DYNAMIC_FAST = 5,
    COL_FILTER_DYNAMIC_ALL = 6,
    COL_FILTER_PHYSICS_VOLUME_ALL = 7,
    COL_FILTER_PHYSICS_VOLUME_F = 8,
    COL_FILTER_PHYSICS_VOLUME_D = 9,
    COL_FILTER_PHYSICS_VOLUME_K = 10,
    COL_FILTER_PHYSICS_VOLUME_DK = 11,
    COL_FILTER_PHYSICS_VOLUME_DF = 12,
    COL_FILTER_PHYSICS_VOLUME_KF = 13,
    COL_FILTER_PLAYER = 14,
    COL_FILTER_PLAYER_CAST = 15,
    COL_FILTER_VOLUME_DETECTABLE = 16,
    COL_FILTER_LIGHTNING_CAST = 17,
    COL_FILTER_RAYCAST_GROUND = 18
};

class DetectionVolumeListener {
public:
    virtual ~DetectionVolumeListener() {}
    virtual void OnCollidableEnter(Hmx::Object *, ObjectDir *) = 0;
    virtual void OnCollidableExit(Hmx::Object *, ObjectDir *) = 0;
};

class DetectionVolume : public Hmx::Object {
public:
    virtual ~DetectionVolume() {}
    virtual void SetActiveState(bool) = 0;
    virtual bool GetActiveState() const = 0;
    virtual void Reset(const Transform &) = 0; // 0x60
    virtual void ApplyRadialForce(float) {} // 0x64
    virtual void ApplyTangentialForce(Vector3) {} // 0x68
    virtual void ApplyDirectionalForce(const Vector3 &) {} // 0x6c
    virtual void ApplyDirectionalImpulse(const Vector3 &) {} // 0x70 - tentative
    virtual void ApplyRadialImpulse(float) {} // 0x74 - tentative
    virtual void ApplyDirectionalLinearVelocity(const Vector3 &) {} // 0x78
    virtual void SetCollisionFilter(CollisionFilter) {} // 0x7c
    virtual void GetOverlaps(std::list<std::pair<Hmx::Object *, ObjectDir *> > &) {
    } // 0x80

protected:
    DetectionVolumeListener *mListener; // 0x2c
};

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
    void CreatePhysicsVolume(class PhysicsManager *);
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
    PhysicsVolumeType mShapeType; // 0x120
    int mOverlapping;
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
