#include "world/PhysicsVolume.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"

PhysicsVolume::PhysicsVolume()
    : mDetectionVolume(nullptr), mShapeType(kPhysicsVolumeBox), unk124(0),
      mDirectionalForce(Vector3::ZeroVec()), mTangentialForce(Vector3::ZeroVec()),
      mDirectionalVelocity(Vector3::ZeroVec()), mRadialForce(0),
      mFilter(kCollidePhysicsVolumeDynamicFixed), mActive(true),
      mReportOnOverlaps(false) {}

PhysicsVolume::~PhysicsVolume() { DestroyPhysicsVolume(); }

BEGIN_HANDLERS(PhysicsVolume)
    HANDLE(set_directional_force, OnSetDirectionalForce)
    HANDLE(set_directional_velocity, OnSetDirectionalVelocity)
    HANDLE(set_tangential_force, OnSetTangentialForce)
    HANDLE(iterate_overlaps, OnIterateOverlaps)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(PhysicsVolume)
    SYNC_PROP_SET(active, mActive, SetActiveState(_val.Int()))
    SYNC_PROP(report_on_overlaps, mReportOnOverlaps)
    SYNC_PROP_SET(
        collision_filter, (int &)mFilter, mFilter = (CollisionFilter)_val.Int();
        if (mDetectionVolume) mDetectionVolume->SetCollisionFilter(mFilter)
    )
    SYNC_PROP(radial_force, mRadialForce)
    SYNC_PROP(directional_force, mDirectionalForce)
    SYNC_PROP(tangential_force, mTangentialForce)
    SYNC_PROP(directional_velocity, mDirectionalVelocity)
    SYNC_PROP_SET(shape_type, mShapeType, ChangeShapeType(_val.Int()))
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(RndPollable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS
