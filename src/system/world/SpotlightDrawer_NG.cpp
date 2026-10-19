#include "world/SpotlightDrawer_NG.h"
#include "macros.h"
#include "math/Color.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/Memory.h"
#include "os/Timer.h"
#include "rnddx9/Rnd.h"
#include "rnddx9/Tex.h"
#include "rndobj/Draw.h"
#include "rndobj/HiResScreen.h"
#include "rndobj/RenderState.h"
#include "rndobj/Cam.h"
#include "rndobj/Rnd.h"
#include "rndobj/Rnd_NG.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/Tex.h"
#include "utl/Loader.h"
#include "world/Dir.h"
#include "world/Spotlight.h"
#include "world/SpotlightDrawer.h"
#include "xdk/d3d9i/d3d9.h"
#include "xdk/d3d9i/d3d9types.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/winerror.h"

static float kFogIllumScale = 0.1f;
static float thickness = 1.0f;

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
    Hmx::Color amb = color;
    sEnviron->SetAmbientColor(color);
    Vector4 vec(amb.red, amb.green, amb.blue, amb.alpha);
    TheShaderMgr.SetVConstant(kVShader_AmbientCol, vec);
    Vector4 v4(amb.red, amb.green, amb.blue, amb.alpha);
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
    TheRenderState.SetTextureClamp(0xB, RndRenderState::kClamp);
    TheRenderState.SetTextureFilter(0xB, RndRenderState::kLinear, false);
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
        TheRenderState.SetTextureClamp(5, RndRenderState::kClamp);
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

bool NgSpotlightDrawer::CheckCam() {
    mHoldCam = RndCam::Current();
    RndCam *cam;
    if (TheLoadMgr.EditMode()) {
        cam = RndCam::Current();
    } else if (TheWorld && TheWorld->Cam()) {
        cam = TheWorld->Cam();
    } else {
        cam = !RndCam::Current() ? TheRnd.GetDefaultCam() : RndCam::Current();
    }
    mCam->Copy(cam, Hmx::Object::kCopyShallow);
    mCam->SetTransParent(nullptr, false);
    return true;
}

void NgSpotlightDrawer::RenderFogProxy() {
    static float fadeDist = 10.0f;
    RndDrawable *proxy = mParams.mProxy;
    if (mParams.mProxy) {
        MILO_ASSERT(mFogDensityMap == SR().mDensityMap, 0x400);
        TheShaderMgr.SetPConstant(kPShader_FogDensityMap, (RndTex *)0);
        Vector4 range(mCam->NearPlane(), -1.0f / mCam->NearPlane(), 1.0f, 0);
        TheShaderMgr.SetVConstant(kVShader_FadeRange, range);
        Vector4 fogVec(
            1.0f / fadeDist, 1.0f, 0, 0 < mCam->FarPlane() ? 1.0f / mCam->FarPlane() : 0
        );
        TheShaderMgr.SetPConstant(kPShader_FogRange, fogVec);
        mCam->SetTargetTex(mFogDensityMap);
        mCam->Select();
        proxy->Draw();
        RestoreCam();
        if (mFogDensityMap) {
            TheShaderMgr.SetPConstant(kPShader_FogDensityMap, mFogDensityMap);
            TheRenderState.SetTextureClamp(5, RndRenderState::kClamp);
        }
    }
}

void NgSpotlightDrawer::RenderSphere(Spotlight *sl) {
    MILO_ASSERT(sl->HasBeam(), 0x470);
    Vector4 fogVec(0, 0, 0.625f, sl->GetBeam().mTopRadius * thickness);
    TheShaderMgr.SetPConstant(kPShader_FogRange, fogVec);

    TheShaderMgr.SetForceCullMode(RndShaderMgr::kNone);

    SetXSectionTexture(sl->GetBeam());
    sl->GetBeam().mBeam->DrawShowing();
}

void NgSpotlightDrawer::RenderScene() {
    Vector4 zRange;
    static const float kFogHalf = 0.125f;
    START_AUTO_TIMER("world_draw");
    sActiveFrame = false;

    int size = sLights.size();
    if (size == 0) {
        ClearPostProc();
        return;
    }

    if (mShowing && CheckSharedResources() && CheckFogTexture()) {
        MILO_ASSERT(sEnviron->GetUseApprox() == false, 0x595);
        sEnviron->Select(nullptr);
        TheShaderMgr.SetPostSpotlight(true);
        TheHiResScreen.SetOverride(true);
        TheRenderState.SetTextureFilter(9, RndRenderState::kPoint, false);
        TheRenderState.SetTextureClamp(9, RndRenderState::kClamp);

        Vector4 screenSize(
            mCam->NearPlane(), mCam->FarPlane(), 1.0f / RTHeight(), 1.0f / RTHeight()
        );
        TheShaderMgr.SetPConstant(kPShader_ScreenSize, screenSize);
        mCam->GetDepthRangeValues(zRange);
        TheShaderMgr.SetPConstant(kPShader_ZRange, zRange);
        TheShaderMgr.SetPConstant(
            kPShader_DepthHalf, Vector4(kFogHalf, kFogHalf, kFogHalf, kFogHalf)
        );
        RenderFogProxy();
        mCam->SetTargetTex(SR().mDepthRT);
        mCam->Select();
        RenderBeams(RndCam::Current()->ViewProj());
        TheRenderState.SetTextureFilter(9, RndRenderState::kPoint, false);
        TheRenderState.SetTextureClamp(9, RndRenderState::kClamp);
        RestoreCam();
        TheHiResScreen.SetOverride(false);
        SetupForPostProcess();
    } else {
        ClearPostProc();
    }
}

void NgSpotlightDrawer::SetupForPostProcess() {
    static float kFogExpand = 32.0f;
    TheShaderMgr.SetPConstant(kPShader_FogColor, Vector4(0, 0, 0, 0));
    BlurRT();
    Vector4 fogVec(
        mParams.mIntensity * kFogExpand,
        mCam->FarPlane(),
        0,
        0 < mCam->FarPlane() ? 1.0f / mCam->FarPlane() : 0
    );
    TheShaderMgr.SetPConstant(kPShader_FogRange, fogVec);
    auto color = mParams.mColor;
    Vector4 fogColor(color.red, color.green, color.blue, color.alpha);
    TheShaderMgr.SetPConstant(kPShader_FogAmbient, fogColor);
    TheShaderMgr.SetPConstant(kPShader_FogDepth, SR().mDepthRT);
    TheRenderState.SetTextureFilter(0xc, RndRenderState::kLinear, false);
    TheRenderState.SetTextureClamp(0xc, RndRenderState::kClamp);
    TheRenderState.SetTextureFilter(5, RndRenderState::kLinear, false);
    TheRenderState.SetTextureClamp(5, RndRenderState::kClamp);
    ClearPostProc();
    sActiveFrame = true;
}

bool NgSpotlightDrawer::CheckRTs(SpotlightResources *sr) {
    PhysMemTypeTracker t("D3D(phys):NgSpotlightDrawer");
    DxTex::SetEDRamChecksEnabled(false);
    int width = RTWidth();
    int height = RTHeight();
    if (!sr->mDepthRT) {
        RndTex *tex = Hmx::Object::New<RndTex>();
        tex->SetBitmap(width, height, 0x20, RndTex::kDepthVolumeMap, false, 0);
        sr->mDepthRT = (DxTex *)tex;
    }
    DxTex::SetEDRamChecksEnabled(true);
    if (!sr->mDepthTex) {
        D3DSURFACE_DESC desc;
        sr->mDepthRT->GetDepthRT()->GetDesc(&desc);
        HRESULT hr = TheDxRnd->CreateTexture(
            RTWidth(), RTHeight(), 1, 0, desc.Format, 0, &sr->mDepthTex, nullptr
        );
        DX_ASSERT(hr, 300);
    }
    if (!sr->mBlack) {
        sr->mBlack = TheRnd.GetDefaultTex(0);
    }
    if (!sr->mWhite) {
        sr->mWhite = TheRnd.GetDefaultTex(2);
    }
    sr->mFarDepth = sr->mBlack;
    if (!sr->mDensityMap) {
        sr->mDensityMap = Hmx::Object::New<RndTex>();
        sr->mDensityMap->SetBitmap(
            RTWidth() >> 1, RTHeight() >> 1, 0x20, RndTex::kDensityMap, false, 0
        );
    }
    return true;
}
