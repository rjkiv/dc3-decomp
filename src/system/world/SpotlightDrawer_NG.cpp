#include "world/SpotlightDrawer_NG.h"
#include "macros.h"
#include "math/Color.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rnddx9/Rnd.h"
#include "rndobj/RenderState.h"
#include "rndobj/Cam.h"
#include "rndobj/Rnd.h"
#include "rndobj/Rnd_NG.h"
#include "rndobj/ShaderMgr.h"
#include "world/Spotlight.h"
#include "world/SpotlightDrawer.h"

void GetLightPosition(Spotlight *s, Vector3 &v) {
    v = s->WorldXfm().v;
    Multiply(v, s->WorldXfm().m, v);
    // clang-format off
    //   if (param_1[0xfd] == 0x0) {
    //     pTVar11 = param_1 + 0x88;
    //   }
    //   else {
    //     pTVar11 = RndTransformable::WorldXfm_Force(param_1 + 0x40);
    //   }
    //   iVar10 = *(param_1 + 0x1f4);
    //   fVar4 = *(iVar10 + 0x80);
    //   fVar5 = *(iVar10 + 0x7c);
    //   fVar6 = *(iVar10 + 0x78);
    // Multiply(V, M, V)
    //   param_2->x += (pTVar11->m).x.x * fVar6 + (pTVar11->m).y.x * fVar5 + (pTVar11->m).z.x * fVar4;
    //   param_2->y += (pTVar11->m).x.y * fVar6 + (pTVar11->m).y.y * fVar5 + (pTVar11->m).z.y * fVar4;
    //   param_2->z += (pTVar11->m).x.z * fVar6 + (pTVar11->m).y.z * fVar5 + (pTVar11->m).z.z * fVar4;
    // clang-format on
}

NgSpotlightDrawer::NgSpotlightDrawer()
    : mCam(), mHoldCam(this), mFogDensityMap(0), mDbgDrawEnable(false) {
    mCam = Hmx::Object::New<RndCam>();
}

NgSpotlightDrawer::~NgSpotlightDrawer() { RELEASE(mCam); }

void NgSpotlightDrawer::EndWorld() {
    if (SpotlightDrawer::sNeedDraw) {
        CheckCam();
    }
    SpotlightDrawer::EndWorld();
}

void NgSpotlightDrawer::DoPost() { RenderScene(); }

void NgSpotlightDrawer::SetAmbientColor(const Hmx::Color &color) {
    Vector4 v4(color.red, color.green, color.blue, color.alpha);
    sEnviron->SetAmbientColor(color);
    TheShaderMgr.SetVConstant(kVShader_AmbientCol, v4);
    TheShaderMgr.SetPConstant(kPShader_AmbientCol, v4);
}

void NgSpotlightDrawer::ClearPostDraw() { sNeedDraw = false; }

void NgSpotlightDrawer::ClearPostProc() {
    sLights.resize(0);
    sShadowSpots.resize(0);
    sCans.resize(0);
}

void NgSpotlightDrawer::Init() {
    CheckSharedResources();
    REGISTER_OBJ_FACTORY(NgSpotlightDrawer);
    RELEASE(sDefault);
    sDefault = Hmx::Object::New<SpotlightDrawer>();
    ((SpotDrawParams &)sDefault->Params()).mLightingInfluence = 0;
    sDefault->Select();
}

int NgSpotlightDrawer::RTWidth() {
    if (TheNgRnd.PreProcessTexture()) {
        return TheNgRnd.PreProcessTexture()->Width() >> 1;
    } else {
        return TheNgRnd.Width() >> 1;
    }
}

int NgSpotlightDrawer::RTHeight() {
    if (TheNgRnd.PreProcessTexture()) {
        return TheNgRnd.PreProcessTexture()->Height() >> 1;
    } else {
        return TheNgRnd.Height() >> 1;
    }
}

void NgSpotlightDrawer::SpotlightResources::Clear() {
    if (mDepthTex) {
        D3DResource_Release(mDepthTex);
        mDepthTex = nullptr;
    }
    RELEASE(mDepthRT);
    RELEASE(mDensityMap);
    mFarDepth = nullptr;
}

void NgSpotlightDrawer::SetXSectionTexture(const Spotlight::BeamDef &def) {
    RndTex *tex = def.mXSection;
    if (!tex) {
        tex = SR().mWhite;
    }
    TheShaderMgr.SetPConstant((PShaderConstant)0xB, tex);
    TheRenderState.SetTextureClamp(0xB, RndRenderState::kClampModeClamp);
    TheRenderState.SetTextureFilter(0xB, RndRenderState::kFilterModeLinear, false);
}

bool NgSpotlightDrawer::RestoreCam() {
    if (mHoldCam) {
        mHoldCam->Select();
    } else {
        TheRnd.GetDefaultCam()->Select();
    }
    return true;
}

bool NgSpotlightDrawer::CheckFogTexture() {
    if (mParams.mProxy) {
        mFogDensityMap = SR().mDensityMap;
    } else if (mParams.mTexture) {
        mFogDensityMap = mParams.mTexture;
    } else {
        mFogDensityMap = SR().mBlack;
    }
    return mFogDensityMap;
}

bool NgSpotlightDrawer::CheckSharedResources() {
    if (sSharedResources) {
        if (sSharedResources->mDepthRT
            && sSharedResources->mDepthRT->Width() != RTWidth()) {
            RELEASE(sSharedResources);
        }
        if (sSharedResources)
            return true;
    }
    sSharedResources = new SpotlightResources();
    return CheckRTs(sSharedResources);
}

void NgSpotlightDrawer::BlurRT() {
    static float kPixBlur = 1.0f;
    static bool kTwoPass = true;

    TheDxRnd.Device()->SetDepthStencilSurface(0);

    if (kTwoPass) {
        BlurRT(kPixBlur, 0.0);
        BlurRT(0.0, kPixBlur);
    } else {
        BlurRT(kPixBlur, kPixBlur);
    }
}

void NgSpotlightDrawer::SetupFogDensityState() {
    Hmx::Matrix4 viewProj;

    if (mFogDensityMap) {
        TheShaderMgr.SetPConstant(kPShader_FogDensityMap, mFogDensityMap);
        TheRenderState.SetTextureClamp(5, RndRenderState::kClampModeClamp);
    }

    RndCam::Current()->GetInfiniteViewProj(viewProj);
    TheShaderMgr.SetVConstant(kVShader_NumLights, viewProj);

    TheShaderMgr.SetPConstant(
        kPShader_FogDensity,
        Vector4(0.0f, mCam->FarPlane() > 0.0f ? 1.0f / mCam->FarPlane() : 0.0f, 0.0f, 0.0f)
    );
}

void NgSpotlightDrawer::SetupFogDensityMap() {
    float x = mParams.mBaseIntensity * 0.01f;
    float y = mParams.mSmokeIntensity * 0.01f;
    TheShaderMgr.SetPConstant(kPShader_FogDensity, Vector4(x, y * (1.0f - x), 0.0f, 0.0f));
}
