#include "rndobj/Draw.h"

#include "os/Debug.h"
#include "utl/BinStream.h"

void RndDrawable::SetShowing(bool show) {
    if (mShowing != show) {
        static Symbol showing("showing");
        mShowing = show;
        BroadcastPropertyChange(showing);
    }
}

DataNode RndDrawable::OnCopySphere(const DataArray *da) {
    RndDrawable *draw = da->Obj<RndDrawable>(2);
    if (draw)
        mSphere = draw->mSphere;
    return 0;
}

DataNode RndDrawable::OnGetSphere(const DataArray *da) {
    *da->Var(2) = mSphere.center.x;
    *da->Var(3) = mSphere.center.y;
    *da->Var(4) = mSphere.center.z;
    *da->Var(5) = mSphere.GetRadius();
    return 0;
}

DataNode RndDrawable::OnSetShowing(const DataArray *da) {
    SetShowing(da->Int(2));
    return 0;
}

DataNode RndDrawable::OnShowing(const DataArray *) { return mShowing; }

DataNode RndDrawable::OnZeroSphere(const DataArray *) {
    mSphere.Zero();
    return 0;
}

void RndDrawable::Save(BinStream &bs) {
    bs << 4;
    bs << (unsigned char)mShowing << mSphere << mOrder;
    bs << mClipPlanes;
}

void RndDrawable::DumpLoad(BinStream &bs) {
    int rev;
    bs >> rev;
    MILO_ASSERT(rev < 4, 0xD6);
    bool bec;
    bs >> bec;
    if (rev < 2) {
        unsigned int count;
        bs >> count;
        while (count != 0) {
            char buf[128];
            bs.ReadString(buf, 128);
            count--;
        }
    }
    if (rev > 0) {
        Sphere s;
        bs >> s;
    }
    if (rev > 2) {
        int x;
        bs >> x;
    }
    if (rev > 3) {
        ObjPtr<RndDrawable> ptr(nullptr);
        bs >> ptr;
    }
}

void RndDrawable::CollideList(
    const Segment &seg, std::list<RndDrawable::Collision> &collisions
) {
    float f;
    Plane pl;
    RndDrawable *draw = Collide(seg, f, pl);
    if (draw) {
        RndDrawable::Collision coll;
        coll.object = draw;
        coll.distance = f;
        coll.plane = pl;
        collisions.push_back(coll);
    }
}

BEGIN_HANDLERS(RndDrawable)
    HANDLE(set_showing, OnSetShowing)
    HANDLE(showing, OnShowing)
    HANDLE(zero_sphere, OnZeroSphere)
    HANDLE_ACTION(update_sphere, UpdateSphere())
    HANDLE(get_sphere, OnGetSphere)
    HANDLE(copy_sphere, OnCopySphere)
    HANDLE(get_draw_children, OnGetDrawChildren)
    HANDLE(get_group_children, OnGetDrawChildren)
END_HANDLERS

RndDrawable::RndDrawable()
    : mShowing(true), mOrder(0), mClipPlanes(this, (EraseMode)0, kObjListNoNull) {
    mSphere.Zero();
}

BEGIN_COPYS(RndDrawable)
    CREATE_COPY(RndDrawable)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax) {
            COPY_MEMBER(mShowing)
            COPY_MEMBER(mOrder)
            COPY_MEMBER(mSphere)
            COPY_MEMBER(mClipPlanes)
        } else {
            if (mSphere.GetRadius() && c->mSphere.GetRadius()) {
                COPY_MEMBER(mSphere);
            }
        }
    END_COPYING_MEMBERS
END_COPYS

BEGIN_PROPSYNCS(RndDrawable)
    SYNC_PROP(draw_order, mOrder)
    SYNC_PROP(showing, mShowing)
    SYNC_PROP(sphere, mSphere)
    SYNC_PROP(clip_planes, mClipPlanes)
END_PROPSYNCS

BEGIN_LOADS(RndDrawable)
    LOAD_REVS(bs)
    ASSERT_REVS(4, 0)
    if (gLoadingProxyFromDisk) {
        bool dummy;
        bs >> dummy;
    } else {
        bsrev >> mShowing;
    }
    if (gRev > 0)
        bs >> mSphere;
    if (gRev > 2) {
        if (gLoadingProxyFromDisk) {
            float dummy;
            bs >> dummy;
        } else
            bs >> mOrder;
    }
    if (gRev > 3)
        bs >> mClipPlanes;
END_LOADS
