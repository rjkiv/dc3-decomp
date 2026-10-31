#pragma once
#include "math/Geo.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "rndobj/Dir.h"
#include "rndobj/Mesh.h"
#include "utl/MemMgr.h"
#include "world/PhysicsVolume.h"

struct RayCast;
class RayCastListener;
class DetectionVolumeListener;
class ContactStateListener;

class RayCastContainer {
public:
    virtual ~RayCastContainer() {}
    virtual Hmx::Object *FindNearest(
        const Segment &seg, float &frac, Vector3 &hitNormal, Hmx::Object *&mesh
    ) = 0;

    MEM_OVERLOAD(RayCastContainer, 0xB8);
};

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
    PhysicsManager(RndDir *parent);
    virtual ~PhysicsManager();
    virtual DataNode Handle(DataArray *, bool);
    virtual void Init() {} // 0x58
    virtual void SyncObjects(bool forceRefresh);
    virtual void Enter() {} // 0x60
    virtual void Poll() {}
    virtual RayCastContainer *MakeContainer(const Box &b, unsigned int filter) = 0;
    virtual DetectionVolume *MakeDetectionVolume(
        DetectionVolumeListener *listener,
        const Transform &worldXfrm,
        PhysicsVolumeType type,
        CollisionFilter filter
    ) = 0;
    virtual void CastRays(
        const Segment *inRay, RayCastListener *rayCallback, int count, unsigned int filter
    ) = 0;
    virtual void CastRays(RayCast *in, int count) = 0;
    virtual bool CastVolume(
        const Segment *cast,
        float radius,
        unsigned int filter,
        Hmx::Object *&hitobj,
        ObjectDir *&hitProxy,
        Vector3 &hitPoint,
        Vector3 &hitNormal
    ) {
        return false;
    } // 0x78
    virtual bool CastVolume(
        const Segment *cast,
        float start_radius,
        float end_radius,
        bool closestToCenter,
        unsigned int filter,
        Hmx::Object *&hitobj,
        ObjectDir *&hitProxy,
        Vector3 &hitPoint,
        Vector3 &hitNormal
    ) {
        return false;
    } // 0x7c
    virtual void MakePhysicsDriven(Hmx::Object *target) {} // 0x80
    virtual void MakeAnimDriven(Hmx::Object *target) {} // 0x84
    virtual void MakeFixed(Hmx::Object *target) {} // 0x88
    virtual void ResetMotionState(Hmx::Object *target) {} // 0x8c
    virtual PhysMotionState GetMotionState(Hmx::Object *target) {
        return kPhysMotion_Fixed;
    } // 0x90
    virtual void SetMotionState(Hmx::Object *target, PhysMotionState state) {} // 0x94
    virtual void ResetMotionType(Hmx::Object *target) {} // 0x98
    virtual void SetCollisionFilter(Hmx::Object *target, CollisionFilter filter) {
    } // 0x9c
    virtual void ApplyForce(Hmx::Object *target, const Vector3 &force) {} // 0xa0
    virtual void SetLinearVelocity(Hmx::Object *target, const Vector3 &vel) {} // 0xa4
    virtual void Move(Hmx::Object *target, const Vector3 &pos, bool unPhysical) {} // 0xa8
    virtual void GetVelocity(Hmx::Object *target, Vector3 &velo4) {} // 0xac
    virtual void GetAngularVelocity(Hmx::Object *target, Vector3 &vel) {} // 0xb0
    virtual float GetMass(Hmx::Object *target) { return 0; } // 0xb4
    virtual void SetMass(Hmx::Object *target, float mass) {} // 0xb8
    virtual float GetForce(Hmx::Object *target) { return 0; } // 0xbc
    virtual void SetGravityFactor(Hmx::Object *target, float g) {} // 0xc0
    virtual void SetFriction(Hmx::Object *target, float f) {} // 0xc4
    virtual void SetRestitution(Hmx::Object *target, float f) {} // 0xc8
    virtual void SetLinearDamp(Hmx::Object *target, float f) {} // 0xcc
    virtual void SetAngularDamp(Hmx::Object *target, float f) {} // 0xd0
    virtual void SetMaxSpeed(Hmx::Object *target, float f) {} // 0xd4
    virtual void SetPenetrationDepth(Hmx::Object *target, float f) {} // 0xd8
    virtual void SetRollingFrictionMult(Hmx::Object *target, float f) {} // 0xdc
    virtual void ResetTrans(Hmx::Object *target) {} // 0xe0
    virtual void SetGravityDirection(const Vector3 &dir) {} // 0xe4
    virtual ObjectDir *GetCollidableDir(Hmx::Object *collidable) {
        return nullptr;
    } // 0xe8
    virtual void AddContactStateListener(ContactStateListener *css) {} // 0xec
    virtual void RemoveContactStateListener(ContactStateListener *css) {} // 0xf0
    virtual bool IsCollidableActive(Hmx::Object *obj) { return false; } // 0xf4
    virtual void ActivateCollidable(Hmx::Object *obj) {} // 0xf8
    virtual void DeactivateCollidable(Hmx::Object *obj) {} // 0xfc

    bool IsShowing(Hmx::Object *o);

protected:
    virtual void RemoveAll() = 0;
    virtual void AddCollidable(Hmx::Object *obj, ObjectDir *parentDir, bool active) = 0;
    virtual void RemoveCollidable(Hmx::Object *obj) = 0;

    DataNode OnGetVelocity(const DataArray *);
    DataNode OnApplyForce(const DataArray *);

private:
    void HarvestCollidables(ObjectDir *parentProxy);

protected:
    RndDir *mWorldDir; // 0x2c
    float mClampTime; // 0x30
    bool mWorldIsBuild; // 0x34
    bool mIsFixedRate; // 0x35
    float mFixedRate; // 0x38
    TaskUnits mTimeUnits; // 0x3c
};

typedef PhysicsManager *PhysicsManagerCreator(RndDir *);
extern PhysicsManagerCreator *CreatePhysicsManager;
