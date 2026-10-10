#include "char/Waypoint.h"
#include "math/Mtx.h"
#include "math/Rand.h"
#include "math/Rot.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Draw.h"
#include "rndobj/Mesh.h"
#include "rndobj/Trans.h"

int waypointunusedlmao() {
    Rand rand(0);
    return rand.Int(0, 69);
}

Waypoint::Waypoint()
    : mFlags(0), mRadius(12), mYRadius(0), mAngRadius(0), mSearchID(0), mStrictAngDelta(0),
      mStrictRadiusDelta(0), mConnections(this, (EraseMode)1) {
    if (RandomFloat() < 0.5f) {
        sWaypoints->push_back(this);
    } else
        sWaypoints->push_front(this);
}

Waypoint::~Waypoint() {
    if (sWaypoints) {
        FOREACH_PTR (it, sWaypoints) {
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

INIT_REVS(5, 0)

BEGIN_LOADS(Waypoint)
    LOAD_REVS(bs)
    ASSERT_REVS(5, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    if (d.rev < 5) {
        RndMesh *mesh = Hmx::Object::New<RndMesh>();
        mesh->RndDrawable::Load(d.stream);
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
    FOREACH_PTR (i, sWaypoints) {
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

void Waypoint::Constrain(Transform &xfm) {
    if (mStrictRadiusDelta > 0) {
        float f1;
        if (mYRadius > 0) {
            f1 = mYRadius + mStrictRadiusDelta;
        } else {
            f1 = 0;
        }
        Vector3 v30;
        ShapeDeltaBox(xfm.v, mRadius + mStrictRadiusDelta, f1, v30);
        xfm.v += v30;
    }
    if (mStrictAngDelta > 0) {
        float f3 = ShapeDeltaAng(mAngRadius + mStrictAngDelta, GetZAngle(xfm.m));
        RotateAboutZ(xfm.m, f3, xfm.m);
    }
}

void Waypoint::ShapeDeltaBox(const Vector3 &v1, float f2, float f3, Vector3 &v4) {
    const Hmx::Matrix3 &mtx = WorldXfm().m;
    if (f3 > 0) {
        Subtract(v1, WorldXfm().v, v4);
        float dot3 = Dot(v4, mtx.x);
        float dot4 = Dot(v4, mtx.y);
        Scale(mtx.x, Clamp(-f2, f2, dot3) - dot3, v4);
        ScaleAddEq(v4, mtx.y, Clamp(-f3, f3, dot4) - dot4);
    } else {
        Subtract(WorldXfm().v, v1, v4);
        v4.z = 0;
        float lensq = LengthSquared(v4);
        if (lensq <= f2 * f2) {
            v4.Zero();
        } else {
            v4 *= 1.0f - (f2 / sqrtf(lensq));
        }
    }
}

Waypoint *Waypoint::FindNearest(const Vector3 &v1, int i2) {
    float dist = kHugeFloat;
    Waypoint *nearest = nullptr;
    FOREACH_PTR (it, sWaypoints) {
        if ((*it)->mFlags & i2) {
            if (MinEq(dist, DistanceSquared(v1, (*it)->WorldXfm().v))) {
                nearest = *it;
            }
        }
    }
    return nearest;
}

DataNode Waypoint::OnWaypointLast(DataArray *a) {
    Waypoint *w = a->Obj<Waypoint>(1);
    FOREACH_PTR (it, sWaypoints) {
        if (*it == w) {
            sWaypoints->splice(sWaypoints->end(), *sWaypoints, it);
            break;
        }
    }
    return 0;
}
