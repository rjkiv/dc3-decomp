#include "rndobj/DOFProc_NG.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/DOFProc.h"
#include "rndobj/PostProc.h"
#include "rndobj/RenderState.h"
#include "rndobj/Rnd.h"
#include "rndobj/Rnd_NG.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/ShaderOptions.h"
#include "rndobj/Tex.h"
#include "ui/UI.h"

void SetVHBlurWeights(bool b1, int i2, int i3) {
    float constants[] = { 0.125f, 0.125f, 0.125f, 0.125f, 0.125f, 0.125f, 0.125f, 0.125f };
    Vector2 *floatsToUse;
    if (b1) {
        static Vector2 sFloats1[] = {
            Vector2(-0.94201624f, -0.39906216f), Vector2(0.9455861f, -0.76890725f),
            Vector2(-0.0941841f, -0.9293887f),   Vector2(0.34495938f, 0.2938776f),
            Vector2(-0.9158858f, 0.45771432f),   Vector2(-0.8154423f, -0.87912464f),
            Vector2(-0.38277543f, 0.27676845f),  Vector2(0.974844f, 0.7564838f)
        };
        floatsToUse = sFloats1;
    } else {
        static Vector2 sFloats2[] = {
            Vector2(0.44323325f, -0.97511554f), Vector2(0.5374298f, -0.4737342f),
            Vector2(-0.2649691f, -0.41893023f), Vector2(0.79197514f, 0.19090188f),
            Vector2(-0.2418884f, 0.99706507f),  Vector2(-0.81409955f, 0.9143759f),
            Vector2(0.19984126f, 0.78641367f),  Vector2(0.14383161f, -0.1410079f)
        };
        floatsToUse = sFloats2;
    }
    static float sScale = 0.666f;
    float blurScale = RndPostProc::DOFOverrides().GetBlurWidthScale() * sScale;
    TheShaderMgr.SetNumTaps(8);
    float f22 = (float)i2 * blurScale * 4.8828124e-06f;
    float f21 = (float)i3 * blurScale * 1.5432099e-05f;
    for (int i = 0; i < 8; i++) {
        Vector2 cur = floatsToUse[i];
        TheShaderMgr.SetPConstant(
            (PShaderConstant)(0x8A + i), Vector4(cur.x * f22 * 5, cur.y * f21 * 5, 1, 1)
        );
        TheShaderMgr.SetPConstant(
            (PShaderConstant)(0x9A + i),
            Vector4(constants[i], constants[i], constants[i], constants[i])
        );
    }
}

NgDOFProc::NgDOFProc()
    : mEnabled(0), unk34(1), unk38(0), mFocalPlane(1), mBlurDepth(1), mMinBlur(0),
      mMaxBlur(1) {
    TheRnd.RegisterPostProcessor(this);
    MILO_ASSERT(TheNgRnd.PreProcessTexture(), 0x41);
    int w = TheNgRnd.PreProcessTexture()->Width() >> 2;
    int h = TheNgRnd.PreProcessTexture()->Height() >> 2;
    mBlurTex[0] = Hmx::Object::New<RndTex>();
    mBlurTex[0]->SetBitmap(w, h, TheRnd.Bpp(), RndTex::kRenderedNoZ, false, nullptr);
    mBlurTex[1] = mBlurTex[0];
}

NgDOFProc::~NgDOFProc() {
    RELEASE(mBlurTex[0]);
    TheRnd.UnregisterPostProcessor(this);
}

void NgDOFProc::Set(
    const RndCam *cam, float focalPlane, float blurDepth, float maxBlur, float minBlur
) {
    MILO_ASSERT(cam, 0xBF);
    mFocalPlane = focalPlane;
    mBlurDepth =
        Min(RndPostProc::DOFOverrides().GetDepthScale() * blurDepth
                + RndPostProc::DOFOverrides().GetDepthOffset(),
            0.0f);
    mMaxBlur = Clamp(
        0.0f,
        1.0f,
        RndPostProc::DOFOverrides().GetMaxBlurScale() * maxBlur
            + RndPostProc::DOFOverrides().GetMaxBlurOffset()
    );
    mMinBlur = Clamp(
        0.0f,
        1.0f,
        RndPostProc::DOFOverrides().GetMinBlurScale() * minBlur
            + RndPostProc::DOFOverrides().GetMinBlurOffset()
    );
    if (mMaxBlur > 0 && TheUI->IsGameScreenActive()) {
        mEnabled = true;
    }
    if (mBlurDepth <= 0.001f) {
        mBlurDepth = 0.001f;
    }

    float near = cam->NearPlane();
    float far = cam->FarPlane();

    float f6;
    if (mFocalPlane < near) {
        f6 = 0;
    } else {
        float t = -((far / mFocalPlane) * near - far) / (far - near);
        f6 = Interp(cam->GetZRange().x, cam->GetZRange().y, t);
    }
    unk34 = f6;

    float f3 = -(mFocalPlane * mBlurDepth - mFocalPlane);
    if (f3 < near) {
        f6 = 0;
    } else {
        float t = -((far / f3) * near - far) / (far - near);
        f6 = Interp(cam->GetZRange().x, cam->GetZRange().y, t);
    }
    unk38 = f6;
    if (unk34 < unk38 + 0.001f) {
        unk34 = unk38 + 0.001f;
    }
}

void NgDOFProc::DoPost() {
    static DataNode &n = DataVariable("disable_dof");

    if (!n.Int() && TheNgRnd.PreProcessTexture() && TheNgRnd.PreDepthTexture()) {
        MILO_ASSERT(mBlurTex[0] && mBlurTex[1], 0x10E);
        bool ngEnabled = mEnabled;
        if (ngEnabled) {
            unsigned int w = mBlurTex[0]->Width();
            unsigned int h = mBlurTex[0]->Height();
            Hmx::Rect r100(0, 0, w, h);
            NgRnd::Viewport old = TheNgRnd.GetViewport();

            NgRnd::Viewport tmp;
            tmp.mWidth = w;
            tmp.mHeight = h;
            tmp.mMaxZ = 1;
            TheNgRnd.SetViewport(tmp);

            float x = 1 / (unk34 - unk38);
            TheShaderMgr.SetPConstant(
                (PShaderConstant)0x18,
                Vector4(
                    x,
                    -(unk34 * x),
                    Min(mMaxBlur, mMinBlur),
                    mMaxBlur < 0.0f ? 1.0f : mMaxBlur
                )
            );

            RndMat *work = TheShaderMgr.GetWork();
            mBlurTex[0]->MakeDrawTarget();
            work->SetDiffuseTex(TheNgRnd.PreProcessTexture());
            work->SetZMode(kZModeDisable);
            work->SetBlend(BaseMaterial::kBlendSrc);
            work->SetTexWrap(kTexWrapClamp);
            TheNgRnd.DrawRect(
                r100, work, kDownsample4xShader, Hmx::Color(), nullptr, nullptr
            );
            mBlurTex[0]->FinishDrawTarget();
            mBlurTex[1]->MakeDrawTarget();
            work->SetDiffuseTex(mBlurTex[0]);
            work->SetBlend(BaseMaterial::kBlendSrc);
            work->SetZMode(kZModeDisable);
            work->SetTexWrap(kTexWrapClamp);
            SetVHBlurWeights(false, w, h);
            TheNgRnd.DrawRect(r100, work, kBlurShader, Hmx::Color(), nullptr, nullptr);
            mBlurTex[1]->FinishDrawTarget();
            mBlurTex[0]->MakeDrawTarget();
            work->SetDiffuseTex(mBlurTex[1]);
            SetVHBlurWeights(true, w, h);
            TheNgRnd.DrawRect(r100, work, kBlurShader, Hmx::Color(), nullptr, nullptr);
            mBlurTex[0]->FinishDrawTarget();
            TheShaderMgr.SetNumTaps(1);
            TheShaderMgr.SetPConstant((PShaderConstant)8, mBlurTex[0]);
            TheRenderState.SetTextureFilter(8, RndRenderState::kFilterModeLinear, false);
            TheRenderState.SetTextureClamp(8, RndRenderState::kClampModeClamp);
            TheNgRnd.SetViewport(old);
        }
        TheShaderMgr.SetUnk26(ngEnabled);
    }
}

void NgDOFProc::Init() {
    REGISTER_OBJ_FACTORY(NgDOFProc);
    if (TheDOFProc && !dynamic_cast<NgDOFProc *>(TheDOFProc)) {
        RELEASE(TheDOFProc);
        TheDOFProc = Hmx::Object::New<DOFProc>();
        MILO_ASSERT(dynamic_cast< NgDOFProc* >(TheDOFProc) != NULL, 0x175);
        static DataNode &n = DataVariable("the_dof_proc");
        n = TheDOFProc;
    }
}

void NgDOFProc::Terminate() {
    RELEASE(TheDOFProc);
    static DataNode &n = DataVariable("the_dof_proc");
    n = NULL_OBJ;
}
