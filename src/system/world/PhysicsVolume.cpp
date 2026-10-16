#include "world/PhysicsVolume.h"
#include "math/Color.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "rndobj/Utl.h"
#include "world/PhysicsManager.h"
#include "math/Geo.h"
#include "math/Rot.h"
#include "math/Vec.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"

namespace {
    Box gPhysicsVolumeBox(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));
}

PhysicsVolume::PhysicsVolume()
    : mVolume(nullptr), mShapeType(kPhysicsVolumeBox), mOverlapping(0),
      mDirectionalForce(Vector3::GetZero()), mTangentialForce(Vector3::GetZero()),
      mDirectionalVelocity(Vector3::GetZero()), mRadialForce(0),
      mCollisionFilter(COL_FILTER_PHYSICS_VOLUME_DF), mActive(true),
      mReportOverlaps(false) {}

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

void PhysicsVolume::SetCollisionFilter(CollisionFilter cf) {
    mCollisionFilter = cf;
    if (mVolume) {
        mVolume->SetCollisionFilter(mCollisionFilter);
    }
}

BEGIN_PROPSYNCS(PhysicsVolume)
    SYNC_PROP_SET(active, mActive, SetActiveState(_val.Int()))
    SYNC_PROP(report_on_overlaps, mReportOverlaps)
    SYNC_PROP_SET(
        collision_filter,
        (int &)mCollisionFilter,
        SetCollisionFilter((CollisionFilter)_val.Int())
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

BEGIN_SAVES(PhysicsVolume)
    SAVE_REVS(7, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndTransformable)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mActive;
    bs << mRadialForce;
    bs << mDirectionalForce;
    bs << mDirectionalVelocity;
    bs << mShapeType;
    bs << mReportOverlaps;
    bs << mCollisionFilter;
    bs << mTangentialForce;
END_SAVES

BEGIN_COPYS(PhysicsVolume)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndTransformable)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(PhysicsVolume)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mShapeType)
        COPY_MEMBER(mOverlapping)
        COPY_MEMBER(mHalfExtends)
        COPY_MEMBER(mDirectionalForce)
        COPY_MEMBER(mTangentialForce)
        COPY_MEMBER(mDirectionalVelocity)
        COPY_MEMBER(mRadialForce)
        COPY_MEMBER(mActive)
        COPY_MEMBER(mReportOverlaps)
        COPY_MEMBER(mCollisionFilter)
    END_COPYING_MEMBERS
    DestroyPhysicsVolume();
END_COPYS

INIT_REVS(7, 0)

BEGIN_LOADS(PhysicsVolume)
    LOAD_REVS(bs)
    ASSERT_REVS(7, 0)
    if (d.rev < 1) {
        LOAD_SUPERCLASS(RndTransformable)
        LOAD_SUPERCLASS(RndDrawable)
        LOAD_SUPERCLASS(RndPollable)
        LOAD_SUPERCLASS(Hmx::Object)
    } else {
        LOAD_SUPERCLASS(RndPollable)
        LOAD_SUPERCLASS(RndTransformable)
        LOAD_SUPERCLASS(RndDrawable)
    }
    d >> mActive;
    if (d.rev > 1) {
        d >> mRadialForce;
        d >> mDirectionalForce;
    }
    if (d.rev > 2) {
        d >> mDirectionalVelocity;
    }
    if (d.rev > 3) {
        d >> (int &)mShapeType;
    }
    if (d.rev > 4) {
        d >> mReportOverlaps;
    }
    if (d.rev > 5) {
        int filter;
        d >> filter;
        mCollisionFilter = (CollisionFilter)filter;
    }
    if (d.rev > 6) {
        d >> mTangentialForce;
    }
    Vector3 ext;
    HalfExtends(ext);
    Sphere s;
    Vector3 harness_reduction_vector;
    harness_reduction_vector.x = ext.x;
    harness_reduction_vector.y = ext.z;
    harness_reduction_vector.z = ext.y;
    s.radius = Length(harness_reduction_vector);
    s.center = WorldXfm().v;
    SetSphere(s);
END_LOADS

bool PhysicsVolume::MakeWorldSphere(Sphere &s, bool b) {
    Vector3 v;
    HalfExtends(v);
    s.radius = Length(v);
    s.center = WorldXfm().v;
    return true;
}

void PhysicsVolume::DrawShowing() {
    if (sShowing) {
        Vector3 v40;
        Hmx::Color c30;
        if (!mActive) {
            v40.Set(0.5f, 0.5f, 0.5f);
            c30 = Hmx::Color(0.5f, 0.5f, 0.5f);
        } else if (mOverlapping != 0) {
            v40.Set(1, 0, 0);
            c30 = Hmx::Color(1, 0, 0);
        } else {
            v40.Set(1, 1, 0);
            c30 = Hmx::Color(1, 1, 0);
        }
        if (mShapeType == kPhysicsVolumeBox) {
            UtilDrawBox(WorldXfm(), gPhysicsVolumeBox, c30, true);
        } else {
            MakeScale(WorldXfm().m, v40);
            v40 /= 2.0f;
            float len = Length(v40);
            UtilDrawSphere(WorldXfm().v, len, c30, nullptr);
        }
    }
}

void PhysicsVolume::Poll() {
    if (mVolume) {
        if (mVolume->GetActiveState() != mActive) {
            mVolume->SetActiveState(mActive);
        }
        mVolume->Reset(WorldXfm());
        if (mActive) {
            if (mRadialForce > 0) {
                mVolume->ApplyRadialForce(mRadialForce);
            }
            if (LengthSquared(mDirectionalForce) > 0) {
                mVolume->ApplyDirectionalForce(mDirectionalForce);
            }
            if (LengthSquared(mTangentialForce) > 0) {
                mVolume->ApplyTangentialForce(mTangentialForce);
            }
            if (LengthSquared(mDirectionalVelocity) > 0) {
                mVolume->ApplyDirectionalLinearVelocity(mDirectionalVelocity);
            }
        }
        if (mOverlapping) {
            if (mReportOverlaps) {
                static Symbol while_has_overlaps("while_has_overlaps");
                Handle(Message(while_has_overlaps, this), false);
            }
        }
    }
}

void PhysicsVolume::Enter() {
    SetActiveState(mActive);
    RndPollable::Enter();
}

void PhysicsVolume::OnCollidableEnter(Hmx::Object *object, ObjectDir *dir) {
    static Symbol object_enter("object_enter");
    Handle(Message(object_enter, this, object, dir), false);
    mOverlapping++;
}

void PhysicsVolume::OnCollidableExit(Hmx::Object *object, ObjectDir *dir) {
    static Symbol object_exit("object_exit");
    Handle(Message(object_exit, this, object, dir), false);
    if (--mOverlapping < 0) {
        mOverlapping = 0;
    }
}

void PhysicsVolume::HalfExtends(Vector3 &v) {
    MakeScale(WorldXfm().m, v);
    v /= 2;
}

void PhysicsVolume::ChangeShapeType(int t) {
    if (mShapeType != t) {
        DestroyPhysicsVolume();
        mShapeType = (PhysicsVolumeType)t;
    }
}

void PhysicsVolume::SetActiveState(bool active) {
    if (mVolume) {
        if (active) {
            mVolume->Reset(WorldXfm());
        }
        mVolume->SetActiveState(active);
    }
    mActive = active;
}

void PhysicsVolume::CreatePhysicsVolume(PhysicsManager *mgr) {
    if (!mVolume) {
        Vector3 v;
        HalfExtends(v);
        Sphere s;
        s.radius = Length(v);
        s.center = WorldXfm().v;
        SetSphere(s);
        mVolume =
            mgr->MakeDetectionVolume(this, WorldXfm(), mShapeType, mCollisionFilter);
        SetActiveState(mActive);
    }
}

void PhysicsVolume::DestroyPhysicsVolume() {
    if (mVolume)
        delete mVolume;
    mVolume = nullptr;
}

DataNode PhysicsVolume::OnSetDirectionalForce(const DataArray *args) {
    MILO_ASSERT(args->Size() == 5, 0x180);
    Vector3 v(args->Float(2), args->Float(3), args->Float(4));
    mDirectionalForce = v;
    return 0;
}

DataNode PhysicsVolume::OnSetTangentialForce(const DataArray *args) {
    MILO_ASSERT(args->Size() == 5, 0x187);
    Vector3 v(args->Float(2), args->Float(3), args->Float(4));
    mTangentialForce = v;
    return 0;
}

DataNode PhysicsVolume::OnSetDirectionalVelocity(const DataArray *args) {
    MILO_ASSERT(args->Size() == 5, 0x18E);
    Vector3 v(args->Float(2), args->Float(3), args->Float(4));
    mDirectionalVelocity = v;
    return 0;
}

DataNode PhysicsVolume::OnIterateOverlaps(const DataArray *args) {
    if (!mVolume->GetActiveState()) {
        return 0;
    } else {
        std::list<std::pair<Hmx::Object *, ObjectDir *> > pairs;
        mVolume->GetOverlaps(pairs);
        if (pairs.empty()) {
            return 0;
        } else {
            DataNode *var2 = args->Var(2);
            DataNode *var3 = args->Var(3);
            DataNode n2(*var2);
            DataNode n3(*var3);
            FOREACH (it, pairs) {
                *var2 = it->first;
                *var3 = it->second;
                for (int i = 4; i < args->Size(); i++) {
                    args->Command(i)->Execute();
                }
            }
            *var2 = n2;
            *var3 = n3;
            return 0;
        }
    }
}
