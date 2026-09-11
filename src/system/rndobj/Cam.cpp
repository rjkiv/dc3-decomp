#include "rndobj/Cam.h"
#include "Rnd.h"
#include "Utl.h"
#include "math/Mtx.h"
#include "math/Rot.h"

#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/Draw.h"
#include "rndobj/Trans.h"

RndCam *RndCam::sCurrent;
float RndCam::sDefaultNearPlane = 1;
float RndCam::sMaxFarNearPlaneRatio = 1000;
static Transform sFlipYZ(Hmx::Matrix3(1, 0, 0, 0, 0, 1, 0, 1, 0), Vector3(0, 0, 0));

RndCam::RndCam()
    : mNearPlane(sDefaultNearPlane), mFarPlane(mNearPlane * sMaxFarNearPlaneRatio),
      mYFov(0.6024178), unk2cc(1), mZRange(0.0f, 1.0f),
      mScreenRect(0.0f, 0.0f, 1.0f, 1.0f), mTargetTex(this), unk300(Hmx::Matrix4::ID()),
      unk340(Hmx::Matrix4::ID()) {
    UpdateLocal();
}

RndCam::~RndCam() {
    if (sCurrent == this) {
        sCurrent = nullptr;
    }
}

BEGIN_HANDLERS(RndCam)
    HANDLE(set_frustum, OnSetFrustum)
    HANDLE(set_z_range, OnSetZRange)
    HANDLE(far_plane, OnFarPlane)
    HANDLE(set_screen_rect, OnSetScreenRect)
    HANDLE(world_to_screen, OnWorldToScreen)
    HANDLE(screen_to_world, OnScreenToWorld)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndCam)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_PROP_SET(near_plane, mNearPlane, SetFrustum(_val.Float(), mFarPlane, mYFov, 1))
    SYNC_PROP_SET(far_plane, mFarPlane, SetFrustum(mNearPlane, _val.Float(), mYFov, 1))
    SYNC_PROP_SET(
        y_fov,
        mYFov * RAD2DEG,
        SetFrustum(mNearPlane, mFarPlane, _val.Float() * DEG2RAD, 1)
    )
    SYNC_PROP(z_range, mZRange)
    SYNC_PROP_MODIFY(screen_rect, mScreenRect, UpdateLocal())
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndCam)
    SAVE_REVS(0xC, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndTransformable)
    bs << mNearPlane << mFarPlane << mYFov;
    bs << mScreenRect << mZRange;
    bs << mTargetTex;
END_SAVES

BEGIN_COPYS(RndCam)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndTransformable)
    CREATE_COPY(RndCam)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax) {
            COPY_MEMBER(mNearPlane)
            COPY_MEMBER(mFarPlane)
            COPY_MEMBER(mYFov)
            COPY_MEMBER(mScreenRect)
            COPY_MEMBER(mZRange)
            COPY_MEMBER(mTargetTex)
        }
    END_COPYING_MEMBERS
    UpdateLocal();
END_COPYS

INIT_REVS(12, 0)

BEGIN_LOADS(RndCam)
    LOAD_REVS(bs)
    ASSERT_REVS(12, 0)
    if (d.rev > 10) {
        LOAD_SUPERCLASS(Hmx::Object)
    }
    LOAD_SUPERCLASS(RndTransformable)
    if (d.rev < 10) {
        RndDrawable::DumpLoad(d.stream);
    }
    if (d.rev == 8) {
        ObjPtrList<Hmx::Object> objList(this, kObjListNoNull);
        int x;
        d >> x >> objList;
    }
    d >> mNearPlane;
    d >> mFarPlane;
    d >> mYFov;
    if (d.rev < 0xC) {
        mYFov = ConvertFov(mYFov, 0.75f);
    }
    if (d.rev < 2) {
        int x;
        d >> x;
    }
    d >> mScreenRect;
    if (d.rev > 0 && d.rev < 3) {
        int x;
        d >> x;
    }
    if (d.rev > 3) {
        d >> mZRange;
    }
    if (d.rev > 4) {
        d >> mTargetTex;
    }
    if (d.rev == 6) {
        int x;
        d >> x;
    }
    UpdateLocal();
END_LOADS

void RndCam::UpdatedWorldXfm() {
    const Transform &xfm = WorldXfm();
    Invert(xfm, mInvWorldXfm);
    Multiply(mLocalFrustum, xfm, mWorldFrustum);
    Multiply(mInvWorldXfm, mLocalProjectXfm, mWorldProjectXfm);
    Multiply(mInvLocalProjectXfm, xfm, mInvWorldProjectXfm);
}

void RndCam::Select() {
    // i'm chalking this up to an msvc-ism, no way hmx would've written this
    RndCam *cur = sCurrent;
    if (cur) {
        if (sCurrent->TargetTex() && cur != this) {
            sCurrent->TargetTex()->FinishDrawTarget();
        }
    }
    WorldXfm();
    sCurrent = this;
    if (TheRnd.GetAspect() != mAspect) {
        UpdateLocal();
    }
}

Transform RndCam::GetInvViewXfm() {
    Transform out;
    Multiply(sFlipYZ, WorldXfm(), out);
    return out;
}

void RndCam::SetViewProj(const Hmx::Matrix4 &mtx) {
    unk300 = mtx;
    Invert(unk300, unk340);
    Transpose(unk340, unk340);
}

void RndCam::SetTargetTex(RndTex *tex) {
    if (sCurrent == this) {
        if (mTargetTex) {
            mTargetTex->FinishDrawTarget();
        }
    }
    mTargetTex = tex;
    UpdateLocal();
}

void RndCam::Init() {
    REGISTER_OBJ_FACTORY(RndCam);
    if (SystemConfig()) {
        DataArray *cfg = SystemConfig("rnd");
        cfg->FindData("cam_default_near_plane", sDefaultNearPlane, true);
        cfg->FindData("cam_max_far_near_ratio", sMaxFarNearPlaneRatio, true);
    }
    DataRegisterFunc("cam_get_default_near_plane", OnGetDefaultNearPlane);
    DataRegisterFunc("cam_get_max_far_near_ratio", OnGetMaxFarNearPlaneRatio);
}

void RndCam::SetFrustum(float near, float far, float yfov, float f4) {
    if (far - 0.0001f > sMaxFarNearPlaneRatio * near) {
        MILO_NOTIFY_ONCE(
            "%s: %f/%f plane ratio exceeds %d",
            Name(),
            far,
            near,
            (int)sMaxFarNearPlaneRatio
        );
        if (far == mFarPlane) {
            near = far / sMaxFarNearPlaneRatio;
        } else {
            far = sMaxFarNearPlaneRatio * near;
        }
    }
    mNearPlane = near;
    mFarPlane = far;
    mYFov = yfov;
    unk2cc = f4;
    UpdateLocal();
}

float RndCam::WorldToScreen(const Vector3 &w, Vector2 &s) const {
    Vector3 v18;
    Multiply(w, mWorldProjectXfm, v18);
    if (v18.z) {
        float scale = 1.0f / v18.z;
        s.x = v18.x * scale;
        s.y = v18.y * scale;
    } else {
        s.x = v18.x;
        s.y = v18.y;
    }
    s.x = (s.x + 1.0f) / 2.0f;
    s.y = (s.y + 1.0f) / 2.0f;
    s.Set(s.x * mScreenRect.w + mScreenRect.x, s.y * mScreenRect.h + mScreenRect.y);
    return v18.z;
}

void RndCam::ScreenToWorld(const Vector2 &v2, float f, Vector3 &vout) const {
    vout.Set(
        (((v2.x - mScreenRect.x) / mScreenRect.w) * 2.0f - 1.0f) * f,
        (((v2.y - mScreenRect.y) / mScreenRect.h) * 2.0f - 1.0f) * f,
        f
    );
    Multiply(vout, mInvWorldProjectXfm, vout);
}

void RndCam::GetDepthRangeValues(Vector4 &v) const {
    float zx = mZRange.x;
    float zratio = 1.0f / (mZRange.y - zx);
    v.Set(mNearPlane, mFarPlane, zratio, zratio * zx);
}

void RndCam::GetInfiniteViewProj(Hmx::Matrix4 &m4) const {
    Transform tfa0;
    Hmx::Matrix4 me0;
    GetViewProjectXfms(tfa0, me0);
    me0.m[2].z = 1;
    me0.m[3].z = -mNearPlane;
    m4 = tfa0 * me0;
}

void RndCam::GetCamFrustum(Vector3 &v1, Vector3 (&varr)[4]) {
    v1 = WorldXfm().v;
    static Vector2 sV2s[] = { Vector2(0, 0), Vector2(0, 1), Vector2(1, 0), Vector2(1, 1) };
    for (int i = 0; i < DIM(sV2s); i++) {
        ScreenToWorld(sV2s[i], mFarPlane, varr[i]);
        Subtract(varr[i], v1, varr[i]);
    }
}

void RndCam::UpdateLocal() {
    float f2 = (mScreenRect.h / mScreenRect.w) * unk2cc;
    if (mTargetTex) {
        f2 *= (float)mTargetTex->Height() / (float)mTargetTex->Width();
    } else {
        f2 *= TheRnd.YRatio();
    }
    mLocalFrustum.Set(mNearPlane, mFarPlane, mYFov, f2);
    mLocalProjectXfm.Zero();
    mInvLocalProjectXfm.Zero();
    if (!mYFov) {
        mLocalProjectXfm.m.x.x = 1;
        mLocalProjectXfm.m.z.y = -1 / f2;
        mInvLocalProjectXfm.m.x.x = 1;
        mInvLocalProjectXfm.m.y.z = -f2;
    } else {
        float tanned = tanf(mYFov / 2);
        mLocalProjectXfm.m.x.x = f2 / tanned;
        mLocalProjectXfm.m.y.z = 1;
        mLocalProjectXfm.m.z.y = -1 / tanned;
        mInvLocalProjectXfm.m.x.x = tanned / f2;
        mInvLocalProjectXfm.m.y.z = -tanned;
        mInvLocalProjectXfm.m.z.y = 1;
    }
    UpdatedWorldXfm();
    mAspect = TheRnd.GetAspect();
}

DataNode RndCam::OnGetDefaultNearPlane(DataArray *) { return sDefaultNearPlane; }
DataNode RndCam::OnGetMaxFarNearPlaneRatio(DataArray *) { return sMaxFarNearPlaneRatio; }

DataNode RndCam::OnSetFrustum(const DataArray *da) {
    float nearPlane, farPlane, yFov, temp;
    static Symbol near_plane("near_plane");
    static Symbol far_plane("far_plane");
    static Symbol y_fov("y_fov");
    if (!da->FindData(near_plane, nearPlane, false))
        nearPlane = mNearPlane;
    if (!da->FindData(far_plane, farPlane, false))
        farPlane = mFarPlane;
    if (da->FindData(y_fov, yFov, false))
        temp = yFov * DEG2RAD;
    else
        temp = mYFov;
    yFov = temp;
    SetFrustum(nearPlane, farPlane, yFov, 1.0f);
    return 0;
}

DataNode RndCam::OnSetZRange(const DataArray *da) {
    SetZRange(da->Float(2), da->Float(3));
    return 0;
}

DataNode RndCam::OnSetScreenRect(const DataArray *da) {
    Hmx::Rect r(da->Float(2), da->Float(3), da->Float(4), da->Float(5));
    SetScreenRect(r);
    return 0;
}

DataNode RndCam::OnFarPlane(const DataArray *) { return mFarPlane; }

DataNode RndCam::OnWorldToScreen(const DataArray *a) {
    Vector3 w(a->Float(2), a->Float(3), a->Float(4));
    Vector2 s;
    float ret = WorldToScreen(w, s);
    *a->Var(5) = s.x;
    *a->Var(6) = s.y;
    return ret;
}

DataNode RndCam::OnScreenToWorld(const DataArray *a) {
    Vector2 v2(a->Float(2), a->Float(3));
    Vector3 vout;
    ScreenToWorld(v2, a->Float(4), vout);
    *a->Var(5) = vout.x;
    *a->Var(6) = vout.y;
    *a->Var(7) = vout.z;
    return 0;
}
