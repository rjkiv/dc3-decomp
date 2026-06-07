#include "rnddx9/Cam.h"
#include "math/Geo.h"
#include "math/Mtx.h"
#include "math/Utl.h"
#include "os/System.h"
#include "rnddx9/Rnd.h"
#include "rndobj/Cam.h"
#include "rndobj/HiResScreen.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/Stats_NG.h"
#include "rndobj/Tex.h"

DxCam::DxCam() {}

void DxCam::Select() {
    TheNgStats->mCams++;
    RndCam::Select();
    if (mTargetTex) {
        mTargetTex->MakeDrawTarget();
    } else {
        TheDxRnd.MakeDrawTarget();
    }
    Transform xfm;
    Hmx::Matrix4 mtx;
    GetViewProjectXfms(xfm, mtx);
    SetViewport();
    if (mTargetTex) {
        float f1 = 1;
        if (mTargetTex->GetType() != RndTex::kShadowMap) {
            f1 = 0;
        }
        DWORD color = 0;
        DWORD flags = 0;
        if (mTargetTex->GetType() & 0x2 && !(mTargetTex->GetType() & 0x20)) {
            flags |= 0x30;
        }
        if (mTargetTex->GetType() != RndTex::kShadowMap) {
            flags |= 0xF;
        }
        if (mTargetTex->GetType() == RndTex::kDepthVolumeMap) {
            color |= 0xffffffffff000000;
        }

        TheDxRnd.Device()->Clear(0, nullptr, flags, color, f1, 0);
    }
    if (GetGfxMode() == kNewGfx) {
        SetViewProj(xfm * mtx);
        Transform inv = GetInvViewXfm();
        TheShaderMgr.SetVConstant((VShaderConstant)4, unk300);
        TheShaderMgr.SetVConstant((VShaderConstant)16, Hmx::Matrix4(inv));
        Hmx::Rect r = TheHiResScreen.ScreenRect();
        TheShaderMgr.SetVConstant((VShaderConstant)0x46, Vector4(r.x, r.y, r.w, r.h));
        Hmx::Rect r2 = TheHiResScreen.ScreenRect();
        TheShaderMgr.SetPConstant((PShaderConstant)0x46, Vector4(r2.x, r2.y, r2.w, r2.h));
    }
}

unsigned int DxCam::ProjectZ(float f) {
    float fvar1 = Interp(
        mZRange.x,
        mZRange.y,
        (mFarPlane / (mFarPlane - mNearPlane)) * ((f - mNearPlane) / f)
    );
    if (TheDxRnd.Unk301() != 0) {
        fvar1 = 1.0f - fvar1;
    }
    return fvar1 * 16777215.0f;
}
