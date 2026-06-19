#include "rndobj/Line.h"
#include "math/Color.h"
#include "obj/Object.h"
#include "math/Rot.h"
#include "os/Debug.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"

RndLine *gLine;

#pragma region Hmx::Object

RndLine::RndLine()
    : mWidth(1), mHasCaps(true), mLinePairs(false), mFoldAngle(PI / 2), mMat(this),
      mLineUpdate(true) {
    mMesh = Hmx::Object::New<RndMesh>();
    mMesh->SetMutable(0x1F);
    mMesh->SetTransParent(this, false);
    UpdateInternal();
}

BEGIN_HANDLERS(RndLine)
    HANDLE_EXPR(num_points, NumPoints())
    HANDLE_ACTION(
        set_point_pos,
        SetPointPos(_msg->Int(2), Vector3(_msg->Float(3), _msg->Float(4), _msg->Float(5)))
    )
    HANDLE_EXPR(point_color, mPoints[_msg->Int(2)].color.PackAlpha())
    HANDLE_ACTION(
        set_point_color,
        SetPointColor(
            _msg->Int(2),
            Hmx::Color(_msg->Float(3), _msg->Float(4), _msg->Float(5), _msg->Float(6)),
            true
        )
    )
    HANDLE_ACTION(
        set_points_color,
        SetPointsColor(
            _msg->Int(2),
            _msg->Int(3),
            Hmx::Color(_msg->Float(4), _msg->Float(5), _msg->Float(6), _msg->Float(7))
        )
    )
    HANDLE_ACTION(set_update, SetUpdate(_msg->Int(2)))
    HANDLE(set_mat, OnSetMat)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(RndLine::Point)
    SYNC_PROP(point, o.point)
    SYNC_PROP_MODIFY(color, o.color, gLine->UpdatePointColor(_prop->Int(_i - 1), true))
    SYNC_PROP_MODIFY(
        alpha, o.color.alpha, gLine->UpdatePointColor(_prop->Int(_i - 1), true)
    )
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(RndLine)
    gLine = this;
    SYNC_PROP_SET(mat, mMat.Ptr(), SetMat(_val.Obj<RndMat>()))
    SYNC_PROP(width, mWidth)
    SYNC_PROP_SET(fold_angle, mFoldAngle * RAD2DEG, SetFoldAngle(_val.Float() * DEG2RAD))
    SYNC_PROP_MODIFY(has_caps, mHasCaps, SetNumPoints(NumPoints()))
    SYNC_PROP_MODIFY(line_pairs, mLinePairs, SetNumPoints(NumPoints()))
    SYNC_PROP_SET(num_points, NumPoints(), SetNumPoints(_val.Int()))
    SYNC_PROP_MODIFY(points, mPoints, SetNumPoints(NumPoints()))
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const RndLine::Point &pt) {
    bs << pt.point << pt.color;
    return bs;
}

BEGIN_SAVES(RndLine)
    SAVE_REVS(4, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    SAVE_SUPERCLASS(RndTransformable)
    bs << mMat << mPoints << mWidth << mFoldAngle << mHasCaps;
    bs << mLinePairs;
END_SAVES

BEGIN_COPYS(RndLine)
    CREATE_COPY_AS(RndLine, d);
    MILO_ASSERT(d, 0x2D2);
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    COPY_SUPERCLASS(RndTransformable)
    COPY_MEMBER_FROM(d, mMat)
    COPY_MEMBER_FROM(d, mPoints)
    COPY_MEMBER_FROM(d, mWidth)
    COPY_MEMBER_FROM(d, mFoldAngle)
    COPY_MEMBER_FROM(d, mHasCaps)
    COPY_MEMBER_FROM(d, mLinePairs)
    UpdateInternal();
END_COPYS

BinStreamRev &operator>>(BinStreamRev &d, RndLine::Point &pt) {
    d >> pt.point >> pt.color;
    return d;
}

INIT_REVS(4, 0)

BEGIN_LOADS(RndLine)
    LOAD_REVS(bs)
    ASSERT_REVS(4, 0)
    if (d.rev > 3) {
        LOAD_SUPERCLASS(Hmx::Object)
    }
    LOAD_SUPERCLASS(RndDrawable)
    if (d.rev < 3) {
        ObjPtrList<Hmx::Object> objList(this);
        int x;
        d >> x >> objList;
    }
    LOAD_SUPERCLASS(RndTransformable)
    d >> mMat;
    d >> mPoints;
    d >> mWidth;
    if (d.rev > 0) {
        d >> mFoldAngle;
        d >> mHasCaps;
    }
    if (d.rev > 1) {
        d >> mLinePairs;
    }
    UpdateInternal();
END_LOADS

inline TextStream &operator<<(TextStream &ts, const RndLine::Point &pt) {
    ts << "\n\tv:" << pt.point << "\n\tc:" << pt.color;
    return ts;
}

void RndLine::Print() {
    TheDebug << "   points: " << mPoints << "\n";
    TheDebug << "   width: " << mWidth << "\n";
    TheDebug << "   foldAngle: " << mFoldAngle << "\n";
    TheDebug << "   hasCaps: " << mHasCaps << "\n";
    TheDebug << "   linePairs:" << mLinePairs << "\n";
}

#pragma endregion
#pragma region RndDrawable

void RndLine::UpdateSphere() {
    Sphere s;
    MakeWorldSphere(s, true);
    Transform xfm;
    FastInvert(WorldXfm(), xfm);
    Multiply(s, xfm, s);
    SetSphere(s);
}

float RndLine::GetDistanceToPlane(const Plane &p, Vector3 &v3) {
    if (mPoints.empty()) {
        return 0;
    }
    WorldXfm();
    bool first = true;
    float ret = 0;
    FOREACH (it, mPoints) {
        const Vector3 &pt = it->point;
        // stupid but matches
        const Vector3 &planeV = reinterpret_cast<const Vector3 &>(p);
        float dot = Dot(planeV, pt) + p.d;
        if (first || (fabs(dot) < fabs(ret))) {
            first = false;
            v3 = pt;
            ret = dot;
        }
    }
    return ret;
}

bool RndLine::MakeWorldSphere(Sphere &s, bool b2) {
    if (b2) {
        s.Zero();
        FOREACH (it, mPoints) {
            s.GrowToContain(Sphere(it->point, mWidth));
        }
        return true;
    } else {
        if (GetSphere().radius) {
            Multiply(GetSphere(), WorldXfm(), s);
            return true;
        } else
            return false;
    }
}

void RndLine::Mats(std::list<class RndMat *> &mats, bool) {
    if (mMat) {
        mats.push_back(mMat);
    }
}

void RndLine::DrawShowing() {
    if (mPoints.size() >= 2) {
        if (mLineUpdate) {
            RndCam *cam = RndCam::Current();
            UpdateLine(cam->WorldXfm(), cam->NearPlane());
            mMesh->SetWorldXfm(cam->WorldXfm());
        }
        mMesh->DrawShowing();
    }
}

RndDrawable *RndLine::CollideShowing(const Segment &s, float &f, Plane &p) {
    RndDrawable *d = mMesh->Collide(s, f, p);
    return d ? this : d;
}

int RndLine::CollidePlane(const Plane &p) { return mMesh->CollidePlane(p); }

#pragma endregion
#pragma region RndLine

void RndLine::SetMat(RndMat *mat) {
    mMat = mat;
    mMesh->SetMat(mat);
}

void RndLine::SetUpdate(bool b1) {
    mLineUpdate = b1;
    if (!mLineUpdate) {
        Transform xfm(WorldXfm());
        static Vector3 offset(0, -1, 0);
        Multiply(offset, xfm, xfm.v);
        UpdateLine(xfm, 0);
        mMesh->SetLocalPos(offset);
    }
}

void RndLine::SetPointPos(int i, const Vector3 &pos) {
    MILO_ASSERT((i >= 0) && (i < mPoints.size()), 0x1CE);
    mPoints[i].point = pos;
}

void RndLine::SetPointColor(int i, const Hmx::Color &color, bool sync) {
    MILO_ASSERT((i >= 0) && (i < mPoints.size()), 0x1D5);
    mPoints[i].color = color;
    UpdatePointColor(i, sync);
}

void RndLine::UpdatePointColor(int i, bool sync) {
    const Point &pt = mPoints[i];
    VertsMap vmap;
    MapVerts(i, vmap);
    vmap.v++->color = pt.color;
    vmap.v++->color = pt.color;
    if (vmap.t != 0) {
        vmap.v++->color = pt.color;
        vmap.v++->color = pt.color;
    }
    if (sync) {
        mMesh->Sync(0x1F);
    }
}

void RndLine::SetPointsColor(int start, int end, const Hmx::Color &color) {
    MILO_ASSERT((start >= 0) && (start < mPoints.size()) && (end >= 0) && (end < mPoints.size()), 0x1F2);
    if (end < start) {
        std::swap(start, end);
    }
    for (int i = start; i <= end; i++) {
        Point &pt = mPoints[i];
        pt.color = color;
        VertsMap vmap;
        MapVerts(i, vmap);
        vmap.v++->color = color;
        vmap.v++->color = color;
        if (vmap.t != 0) {
            vmap.v++->color = color;
            vmap.v++->color = color;
        }
    }

    mMesh->Sync(0x1F);
}

void RndLine::MapVerts(int idx, VertsMap &map) {
    if (mHasCaps) {
        if (mLinePairs) {
            map.t = (idx & 1) ? 2 : 1;
            map.v = &mMesh->Verts(idx * 4);
        } else if (idx == 0) {
            map.t = 1;
            map.v = mMesh->Verts().begin();
        } else if (idx + 1 == mPoints.size()) {
            map.t = 2;
            auto &verts = mMesh->Verts();
            map.v = &verts[verts.size() - 4];
        } else {
            map.t = 0;
            map.v = &mMesh->Verts((idx + 1) * 2);
        }
    } else {
        map.t = 0;
        map.v = &mMesh->Verts(idx * 2);
    }
}

void RndLine::UpdateInternal() {
    mFoldCos = cos(mFoldAngle);
    mMesh->SetMat(mMat);
    SetNumPoints(mPoints.size());
}

void RndLine::SetNumPoints(int num) {
    mPoints.resize(num);
    if (num >= 1) {
        if (mHasCaps) {
            if (mLinePairs) {
                num = (num & ~1) * 2;
            } else {
                num += 2;
            }
        }
        mMesh->Verts().resize(num * 2);
        for (int i = 0; i < mPoints.size(); i++) {
            VertsMap vmap;
            MapVerts(i, vmap);

            if (vmap.t == 1) {
                vmap.v->tex.Set(0, 1);
                vmap.v++->color = mPoints[i].color;
                vmap.v->tex.Set(0, 0);
                vmap.v++->color = mPoints[i].color;
            }
            vmap.v->tex.Set(0.5f, 1);
            vmap.v++->color = mPoints[i].color;
            vmap.v->tex.Set(0.5f, 0);
            vmap.v++->color = mPoints[i].color;
            if (vmap.t == 2) {
                vmap.v->tex.Set(1, 1);
                vmap.v++->color = mPoints[i].color;
                vmap.v->tex.Set(0, 1);
                vmap.v++->color = mPoints[i].color;
            }
        }

        if (mLinePairs) {
            if (mHasCaps) {
                num = num * 3 >> 1;
            }
        } else {
            num = (num * 2) - 2;
        }
        mMesh->Faces().resize(num);
        for (int i = num - 2; i >= 0; i -= 2) {
            int i8 = i;
            if (mLinePairs) {
                if (mHasCaps) {
                    i8 = i % 6 + (i / 6) * 8;
                } else {
                    i8 = i * 2;
                }
            }
            mMesh->Faces(i).Set(i8, i8 + 2, i8 + 1);
            mMesh->Faces(i + 1).Set(i8 + 1, i8 + 2, i8 + 3);
        }
        mMesh->Sync(0x13F);
    }
}

DataNode RndLine::OnSetMat(const DataArray *array) {
    RndMat *mat = array->Obj<RndMat>(2);
    SetMat(mat);
    SetShowing(mat);
    return 0;
}
