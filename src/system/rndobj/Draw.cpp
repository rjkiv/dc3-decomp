#include "rndobj/Draw.h"
#include "math/Color.h"
#include "math/Mtx.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Cam.h"
#include "rndobj/Env.h"
#include "rndobj/Group.h"
#include "rndobj/Rnd.h"
#include "rndobj/Utl.h"
#include "utl/BinStream.h"

RndDrawable::RndDrawable()
    : mShowing(true), mOrder(0), mClipPlanes(this, (EraseMode)0, kObjListNoNull) {
    mSphere.Zero();
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

BEGIN_PROPSYNCS(RndDrawable)
    SYNC_PROP(draw_order, mOrder)
    SYNC_PROP(showing, mShowing)
    SYNC_PROP(sphere, mSphere)
    SYNC_PROP(clip_planes, mClipPlanes)
END_PROPSYNCS

BEGIN_SAVES(RndDrawable)
    SAVE_REVS(4, 0)
    bs << (unsigned char)mShowing << mSphere << mOrder;
    bs << mClipPlanes;
END_SAVES

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

BEGIN_LOADS(RndDrawable)
    LOAD_REVS(bs)
    ASSERT_REVS(4, 0)
    if (gLoadingProxyFromDisk) {
        bool dummy;
        d >> dummy;
    } else {
        d >> mShowing;
    }
    if (d.rev < 2) {
        unsigned int count;
        bs >> count;
        RndGroup *grp = dynamic_cast<RndGroup *>(this);
        for (; count != 0; count--) {
            char buf[0x80];
            bs.ReadString(buf, 0x80);
            if (grp) {
                Hmx::Object *found = Dir()->Find<Hmx::Object>(buf, true);
                RndCam *cam = dynamic_cast<RndCam *>(found);
                if (!cam) {
                    grp->RemoveObject(found);
                    grp->AddObject(found, 0);
                }
            } else
                MILO_NOTIFY("%s not in group", buf);
        }
    }
    if (d.rev > 0)
        bs >> mSphere;
    if (d.rev > 2) {
        if (gLoadingProxyFromDisk) {
            float dummy;
            bs >> dummy;
        } else
            bs >> mOrder;
    }
    if (d.rev > 3)
        bs >> mClipPlanes;
END_LOADS

void RndDrawable::Draw() {
    if (mShowing) {
        Sphere s;
        if (MakeWorldSphere(s, false) && s > RndCam::Current()->WorldFrustum()) {
            return;
        }
        TheRnd.PushClipPlanes(mClipPlanes);
        DrawShowing();
        TheRnd.PopClipPlanes(mClipPlanes);
    }
}

int RndDrawable::CollidePlane(const Plane &pl) {
    if (mShowing) {
        Sphere s;
        if (MakeWorldSphere(s, false)) {
            if (s >= pl)
                return 1;
            else if (s < pl)
                return -1;
            else
                return 0;
        }
    }
    return -1;
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

void RndDrawable::Highlight() {
    if (sHighlightStyle != kHighlightNone) {
        Sphere s;
        if (MakeWorldSphere(s, false) && s > RndCam::Current()->WorldFrustum()) {
            return;
        }
        bool oldShowing = mShowing;
        mShowing = true;
        UtilDrawSphere(s.center, s.radius, Hmx::Color(1, 1, 0), nullptr);
        mShowing = oldShowing;
    }
}

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

RndDrawable *RndDrawable::Collide(const Segment &seg, float &f, Plane &plane) {
    START_AUTO_TIMER("collide");
    if (!CollideSphere(seg))
        return nullptr;
    else
        return CollideShowing(seg, f, plane);
}

bool RndDrawable::CollideSphere(const Segment &seg) {
    if (!mShowing)
        return false;
    else {
        Sphere sphere;
        if (MakeWorldSphere(sphere, false) && !Intersect(seg, sphere))
            return false;
        else
            return true;
    }
}

DataNode RndDrawable::OnGetDrawChildren(const DataArray *a) {
    bool aSize = a->Size() > 2 ? a->Int(2) : 0;
    std::list<RndDrawable *> drawChildren;
    ListDrawChildren(drawChildren);
    DataArrayPtr ptr(new DataArray(drawChildren.size() + aSize));
    int idx = 0;
    if (aSize) {
        ptr->Node(idx++) = NULL_OBJ;
    }
    for (std::list<RndDrawable *>::iterator it = drawChildren.begin();
         it != drawChildren.end();
         ++it) {
        ptr->Node(idx++) = *it;
    }
    return ptr;
}
