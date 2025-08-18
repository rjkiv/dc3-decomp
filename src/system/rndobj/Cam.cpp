#include "rndobj/Cam.h"
#include "math/Mtx.h"
#include "math/Rot.h"

#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Trans.h"

float RndCam::sDefaultNearPlane = 1;
float RndCam::sMaxFarNearPlaneRatio = 1000;
static Transform sFlipYZ;

void RndCam::UpdatedWorldXfm() {
    const Transform &xfm = WorldXfm();
    Invert(xfm, mInvWorldXfm);
    Multiply(mLocalFrustum, xfm, mWorldFrustum);
    Multiply(mInvWorldXfm, mLocalProjectXfm, mWorldProjectXfm);
    Multiply(mInvLocalProjectXfm, xfm, mInvWorldProjectXfm);
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

RndCam::~RndCam() {
    if (sCurrent == this)
        sCurrent = nullptr;
}

RndCam::RndCam()
    : mNearPlane(sDefaultNearPlane), mFarPlane(mNearPlane * sMaxFarNearPlaneRatio),
      mYFov(0.6024178), unk2cc(1), mZRange(0.0f, 1.0f),
      mScreenRect(0.0f, 0.0f, 1.0f, 1.0f), mTargetTex(this), unk300(Hmx::Matrix4::ID()),
      unk340(Hmx::Matrix4::ID()) {
    UpdateLocal();
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

BEGIN_LOADS(RndCam)
    LOAD_REVS(bs)
    ASSERT_REVS(12, 0)
    if (gRev > 10) {
        LOAD_SUPERCLASS(Hmx::Object)
    }
    LOAD_SUPERCLASS(RndTransformable)
    if (gRev < 10) {
        RndDrawable::DumpLoad(bs);
    }
    if (gRev == 8) {
        int x;
        ObjPtrList<Hmx::Object> objList(this, kObjListNoNull);
        bs >> x >> objList;
    }
//       if (uVar1 > 10) {
//     Hmx::Object::Load(this + *(*(this + -0x380) + 4) + -0x380,param_1);
//   }
//   RndTransformable::Load(this + -0x2c0,param_1);
//   if (uVar1 < 10) {
//     RndDrawable::DumpLoad(param_1);
//   }
//   if (uVar1 == 8) {
//     if (this == 0x384) {
//       pOVar5 = 0x0;
//     }
//     else {
//       pOVar5 = this + *(*(this + -0x380) + 4) + -0x380;
//     }
//     ObjPtrList<>::ObjPtrList<>(&OStack_60,pOVar5,0);
//     BinStream::ReadEndian(param_1,auStack_84,4);
//     ObjPtrList<>::Load(&OStack_60,param_1,true,0x0,true);
//     ObjPtrList<>::~ObjPtrList<>(&OStack_60);
//   }
END_LOADS

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
