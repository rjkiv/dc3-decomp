#include "world/Crowd.h"
#include "char/Character.h"
#include "obj/Object.h"
#include "os/System.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/Poll.h"
#include "rndobj/Tex.h"
#include "rndobj/Utl.h"

RndCam *gImpostorCamera;
RndMat *gImpostorMat;
int gNumCrowd;
WorldCrowd *gParent;
RndTex *gImpostorTex[kNumLods];

WorldCrowd::WorldCrowd()
    : mPlacementMesh(this), mCharacters(this), mNum(0), unk6c(0), mForce3DCrowd(0),
      mShow3DOnly(0), unk84(1), unk88(1), mLod(0), mEnviron(this), mEnviron3D(this),
      mFocus(this), mCharForceLod(kLODPerFrame), unkd0(0), unkd4(0) {
    if (gNumCrowd++ == 0) {
        int w, h, bpp;
        if (GetGfxMode() == kNewGfx) {
            w = 256;
            h = 512;
            bpp = 32;
        } else {
            w = 128;
            h = 256;
            bpp = 16;
        }
        for (int i = 0; i < kNumLods; i++) {
            gImpostorTex[i] = Hmx::Object::New<RndTex>();
            gImpostorTex[i]->SetBitmap(w, h, bpp, RndTex::kTexRendered, true, nullptr);
        }
        RELEASE(gImpostorMat);
        RndMat *mat = Hmx::Object::New<RndMat>();
        gImpostorMat = mat;
        mat->SetUseEnv(true);
        mat->SetPreLit(false);
        mat->SetBlend(RndMat::kBlendSrc);
        mat->SetZMode(kZModeNormal);
        mat->SetAlphaCut(true);
        mat->SetAlphaThreshold(0x80);
        mat->SetTexWrap(kTexWrapClamp);
        mat->SetPerPixelLit(false);
        mat->SetPointLights(true);
        CreateAndSetMetaMat(mat);
        gImpostorCamera = Hmx::Object::New<RndCam>();
        SetMatAndCameraLod();
    }
}

WorldCrowd::~WorldCrowd() {
    Delete3DCrowdHandles();
    for (ObjList<CharData>::iterator it = mCharacters.begin(); it != mCharacters.end();
         ++it) {
        if (it->mMultiMesh) {
            delete it->mMultiMesh->Mesh();
            RELEASE(it->mMultiMesh);
        }
    }
    gNumCrowd--;
    if (gNumCrowd == 0) {
        for (int i = 0; i < kNumLods; i++) {
            RELEASE(gImpostorTex[i]);
        }
        RELEASE(gImpostorCamera);
        RELEASE(gImpostorMat);
    }
}

DataNode WorldCrowd::OnRebuild(DataArray *) { return 0; }

BEGIN_HANDLERS(WorldCrowd)
    HANDLE(rebuild, OnRebuild)
    HANDLE_ACTION(assign_random_colors, AssignRandomColors(true))
    HANDLE(iterate_frac, OnIterateFrac)
    HANDLE_ACTION(set_fullness, SetFullness(_msg->Float(2), _msg->Float(3)))
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(WorldCrowd::CharData)
    SYNC_PROP(character, o.mDef.mChar)
    SYNC_PROP(height, o.mDef.mHeight)
    SYNC_PROP(density, o.mDef.mDensity)
    SYNC_PROP(radius, o.mDef.mRadius)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(WorldCrowd)
    gParent = this;
    SYNC_PROP(num, mNum)
    SYNC_PROP(placement_mesh, mPlacementMesh)
    SYNC_PROP(characters, mCharacters)
    SYNC_PROP(show_3d_only, mShow3DOnly)
    SYNC_PROP(environ, mEnviron)
    SYNC_PROP(environ_3d, mEnviron3D)
    SYNC_PROP_SET(lod, mLod, SetLod(_val.Int()))
    SYNC_PROP_SET(force_3D_crowd, mForce3DCrowd, Force3DCrowd(_val.Int()))
    SYNC_PROP(focus, mFocus)
    SYNC_PROP(char_force_lod, (int &)mCharForceLod)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void WorldCrowd::SetLod(int lod) { mLod = Clamp(0, 2, lod); }
