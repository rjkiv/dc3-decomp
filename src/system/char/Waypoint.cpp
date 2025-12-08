#include "char/Waypoint.h"
#include "math/Rand.h"
#include "math/Rot.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Draw.h"
#include "rndobj/Mesh.h"
#include "rndobj/Trans.h"

Waypoint::Waypoint()
    : mFlags(0), mRadius(12), mYRadius(0), mAngRadius(0), unkd0(0), mStrictAngDelta(0),
      mStrictRadiusDelta(0), mConnections(this, (EraseMode)1) {
    if (RandomFloat() < 0.5f) {
        sWaypoints->push_back(this);
    } else
        sWaypoints->push_front(this);
}

Waypoint::~Waypoint() {
    if (sWaypoints) {
        for (std::list<Waypoint *>::iterator it = sWaypoints->begin();
             it != sWaypoints->end();
             ++it) {
            if (*it == this) {
                sWaypoints->erase(it);
                break;
            }
        }
    }
}

BEGIN_HANDLERS(Waypoint)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(Waypoint)
    SYNC_PROP(flags, mFlags)
    SYNC_PROP(radius, mRadius)
    SYNC_PROP(y_radius, mYRadius)
    SYNC_PROP_SET(ang_radius, mAngRadius * RAD2DEG, mAngRadius = _val.Float() * DEG2RAD)
    SYNC_PROP(strict_radius_delta, mStrictRadiusDelta)
    SYNC_PROP_SET(
        strict_ang_delta,
        mStrictAngDelta * RAD2DEG,
        mStrictAngDelta = _val.Float() * DEG2RAD
    )
    SYNC_PROP(connections, mConnections)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(Waypoint)
    SAVE_REVS(5, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndTransformable)
    bs << mFlags;
    bs << mConnections;
    bs << mRadius;
    bs << mYRadius;
    bs << mAngRadius;
    bs << mStrictRadiusDelta;
    bs << mStrictAngDelta;
END_SAVES

BEGIN_COPYS(Waypoint)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndTransformable)
    CREATE_COPY(Waypoint)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mFlags)
        COPY_MEMBER(mConnections)
        COPY_MEMBER(mRadius)
        COPY_MEMBER(mYRadius)
        COPY_MEMBER(mAngRadius)
        COPY_MEMBER(mStrictRadiusDelta)
        COPY_MEMBER(mStrictAngDelta)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(Waypoint)
    LOAD_REVS(bs)
    ASSERT_REVS(5, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    if (d.rev < 5) {
        RndMesh *mesh = Hmx::Object::New<RndMesh>();
        mesh->RndDrawable::Load(bs);
        if (mesh) {
            delete mesh;
        }
    }
    LOAD_SUPERCLASS(RndTransformable)
    d >> mFlags;
    d >> mConnections;
    if (d.rev > 1) {
        d >> mRadius;
    } else
        mRadius = 12;
    if (d.rev > 2) {
        d >> mYRadius;
        d >> mAngRadius;
    }
    if (d.rev > 3) {
        d >> mStrictRadiusDelta;
        d >> mStrictAngDelta;
    }
END_LOADS

void Waypoint::Init() {
    REGISTER_OBJ_FACTORY(Waypoint);
    DataRegisterFunc("waypoint_find", OnWaypointFind);
    DataRegisterFunc("waypoint_nearest", OnWaypointNearest);
    DataRegisterFunc("waypoint_last", OnWaypointLast);
    sWaypoints = new std::list<Waypoint *>();
    TheDebug.AddExitCallback(Waypoint::Terminate);
}

void Waypoint::Terminate() { RELEASE(sWaypoints); }

Waypoint *Waypoint::Find(int flags2) {
    for (std::list<Waypoint *>::iterator i = sWaypoints->begin(); i != sWaypoints->end();
         ++i) {
        if ((*i)->mFlags & flags2)
            return *i;
    }
    return nullptr;
}

DataNode Waypoint::OnWaypointFind(DataArray *da) { return Waypoint::Find(da->Int(1)); }

DataNode Waypoint::OnWaypointNearest(DataArray *da) {
    return FindNearest(da->Obj<RndTransformable>(1)->WorldXfm().v, da->Int(2));
}

float Waypoint::ShapeDeltaAng(float f1, float f2) {
    float limited = LimitAng(GetZAngle(WorldXfm().m) - f2);
    float clamped = Clamp(-f1, f1, limited);
    return limited - clamped;
}

float Waypoint::ShapeDelta(float f) { return ShapeDeltaAng(mAngRadius, f); }

void Waypoint::ShapeDelta(const Vector3 &v, Vector3 &vout) {
    ShapeDeltaBox(v, mRadius, mYRadius, vout);
}
