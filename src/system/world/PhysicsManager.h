#pragma once
#include "math/Geo.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "world/PhysicsVolume.h"

struct RayCast;
class RayCastContainer;
class RayCastListener;
class DetectionVolume;
class DetectionVolumeListener;
class ContactStateListener;

enum PhysMotionState {
    /** "Things can collide with me, but I don't move." */
    kPhysMotion_Fixed = 0,
    /** "Things can collide with me, and I can be moved by animations." */
    kPhysMotion_Keyframed = 1,
    /** "My motion is fully derived from the physics engine" */
    kPhysMotion_Dynamic = 2,
    kPhysMotion_Initial = 3
};

class PhysicsManager : public Hmx::Object {
public:
    PhysicsManager(RndDir *);
    virtual ~PhysicsManager() {}
    virtual DataNode Handle(DataArray *, bool);
    // there's a LOT of return void stubs here,
    // i can't verify their order
    virtual void Init() {} // 0x58 - tentative
    virtual void SyncObjects(bool);
    virtual void Enter() {} // 0x60 - tentative
    virtual void Poll() {}
    virtual RayCastContainer *MakeContainer(const Box &, unsigned int) = 0;
    virtual DetectionVolume *MakeDetectionVolume(
        DetectionVolumeListener *, const Transform &, PhysicsVolumeType, CollisionFilter
    ) = 0;
    virtual void CastRays(RayCast *, int) = 0;
    virtual void CastRays(const Segment *, RayCastListener *, int, unsigned int) = 0;
    virtual bool
    CastVolume(const Segment *, float, unsigned int, Hmx::Object *&, ObjectDir *&, Vector3 &, Vector3 &) {
        return false;
    } // 0x78
    virtual bool
    CastVolume(const Segment *, float, float, bool, unsigned int, Hmx::Object *&, ObjectDir *&, Vector3 &, Vector3 &) {
        return false;
    } // 0x7c
    virtual void MakePhysicsDriven(Hmx::Object *) {} // 0x80
    virtual void MakeAnimDriven(Hmx::Object *) {} // 0x84

    virtual void Move(Hmx::Object *, const Vector3 &, bool) {} // 0x88 - tentative
    virtual void MakeFixed(Hmx::Object *) {} // 0x8c - tentative

    virtual PhysMotionState GetMotionState(Hmx::Object *) {
        return (PhysMotionState)0;
    } // 0x90

    virtual void SetMotionState(Hmx::Object *, PhysMotionState) {} // 0x94 - tentative
    virtual void ResetMotionState(Hmx::Object *) {} // 0x98 - tentative
    virtual void ResetMotionType(Hmx::Object *) {} // 0x9c - tentative

    virtual void ApplyForce(Hmx::Object *, const Vector3 &) {} // 0xa0

    virtual void SetGravityFactor(Hmx::Object *, float) {} // 0xa4 - tentative
    virtual void SetCollisionFilter(Hmx::Object *, CollisionFilter) {} // 0xa8 - tentative

    virtual void GetVelocity(Hmx::Object *, Vector3 &) {} // 0xac

    virtual void SetLinearVelocity(Hmx::Object *, const Vector3 &) {} // 0xb0 - tentative

    virtual float GetMass(Hmx::Object *) { return 0; } // 0xb4
    virtual void GetAngularVelocity(Hmx::Object *, Vector3 &) {} // 0xb8
    virtual float GetForce(Hmx::Object *) { return 0; } // 0xbc

    virtual void SetAngularDamp(Hmx::Object *, float) {} // 0xc0 - tentative
    virtual void SetPenetrationDepth(Hmx::Object *, float) {} // 0xc4 - tentative
    virtual void SetMaxSpeed(Hmx::Object *, float) {} // 0xc8 - tentative
    virtual void SetFriction(Hmx::Object *, float) {} // 0xcc - tentative
    virtual void SetMass(Hmx::Object *, float) {} // 0xd0 - tentative
    virtual void SetLinearDamp(Hmx::Object *, float) {} // 0xd4 - tentative
    virtual void SetRestitution(Hmx::Object *, float) {} // 0xd8 - tentative
    virtual void SetRollingFrictionMult(Hmx::Object *, float) {} // 0xdc - tentative

    virtual void ResetTrans(Hmx::Object *) {} // 0xe0
    virtual void SetGravityDirection(const Vector3 &) {} // 0xe4
    virtual ObjectDir *GetCollidableDir(Hmx::Object *) { return nullptr; } // 0xe8

    virtual void AddContactStateListener(ContactStateListener *) {} // 0xec - tentative
    virtual void RemoveContactStateListener(ContactStateListener *) {} // 0xf0 - tentative

    virtual bool IsCollidableActive(Hmx::Object *) { return false; } // 0xf4
    virtual void ActivateCollidable(Hmx::Object *) {} // 0xf8
    virtual void DeactivateCollidable(Hmx::Object *) {} // 0xfc
    virtual void RemoveAll() = 0;
    virtual void AddCollidable(Hmx::Object *, ObjectDir *, bool) = 0;
    virtual void RemoveCollidable(Hmx::Object *) = 0;

    bool IsShowing(Hmx::Object *);

private:
    void HarvestCollidables(ObjectDir *);

protected:
    RndDir *unk2c; // 0x2c
    float mPhysicsClampTime; // 0x30
    bool unk34; // 0x34
    bool unk35; // 0x35
    float unk38; // 0x38
    int unk3c; // 0x3c
};
