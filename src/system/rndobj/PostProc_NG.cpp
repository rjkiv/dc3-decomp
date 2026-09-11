#include "rndobj/PostProc_NG.h"

#include "HiResScreen.h"
#include "hamobj/HamDirector.h"
#include "math/Color.h"
#include "math/Geo.h"
#include "math/Mtx.h"
#include "math/Trig.h"
#include "os/Memory.h"
#include "ShaderMgr.h"
#include "Tex.h"
#include "math/Rand.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Overlay.h"
#include "rndobj/RenderState.h"
#include "rndobj/PostProc.h"
#include "rndobj/Rnd.h"
#include "rndobj/Rnd_NG.h"
#include "rndobj/ShaderOptions.h"
#include "rndobj/Tex.h"
#include "rndobj/Utl.h"
#include "rndobj/VelocityBuffer.h"
#include "utl/Loader.h"

NgPostProc *NgPostProc::s_BloomSetter;
Hmx::Color NgPostProc::s_prevBloomColor(-1, -1, -1, -1);
float NgPostProc::s_prevBloomIntensity = -1;
NgPostProc::BloomTextures<3> NgPostProc::sBloom;

NgPostProc::BloomTextureSet::BloomTextureSet() {
    for (int i = 0; i < 2; i++) {
        mBloomTexture[i] = nullptr;
    }
}

NgPostProc::BloomTextureSet::~BloomTextureSet() { FreeTextures(); }

void NgPostProc::BloomTextureSet::AllocateTextures(unsigned int w, unsigned int h) {
    MILO_ASSERT(mBloomTexture[0] == NULL, 0x48);
    mBloomTexture[0] = Hmx::Object::New<RndTex>();
    mBloomTexture[0]->SetBitmap(w, h, TheRnd.Bpp(), RndTex::kRenderedNoZ, false, nullptr);
    mBloomTexture[1] = mBloomTexture[0];
}

void NgPostProc::BloomTextureSet::FreeTextures() { RELEASE(mBloomTexture[0]); }

NgPostProc::NgPostProc()
    : unk22c(RandomFloat(), RandomFloat()), unk234(0, 0), unk23c(this), unk250(1) {}

NgPostProc::~NgPostProc() {}

void NgPostProc::Select() {
    RndPostProc::Select();
    unk22c.x = RandomFloat();
    unk22c.y = RandomFloat();
}

void NgPostProc::OnSelect() {
    RndPostProc::OnSelect();
    unk23c.clear();
}

void NgPostProc::OnUnselect() {
    RndPostProc::OnUnselect();
    unk23c.clear();
}

void NgPostProc::Init() {
    REGISTER_OBJ_FACTORY(NgPostProc);
    PhysMemTypeTracker tracker("D3D(phys):NgPostProc");
    RebuildTex();
}

void NgPostProc::RebuildTex() {
    ReleaseTex();
    unsigned int w = 0x80;
    unsigned int h = 0x80;
    if (TheLoadMgr.GetPlatform() != kPlatformNone) {
        MILO_ASSERT(TheNgRnd.PreProcessTexture(), 0x3AB);
        w = TheNgRnd.PreProcessTexture()->Width();
        h = TheNgRnd.PreProcessTexture()->Height();
    }
    RndVelocityBuffer::Singleton().AllocateData(w, h, TheRnd.Bpp());
    sBloom.AllocateTextures(w, h);
}

void NgPostProc::ReleaseTex() {
    sBloom.FreeTextures();
    RndVelocityBuffer::Singleton().FreeData();
}

void NgPostProc::EndWorld() {
    // scary
    RndVelocityBuffer::Singleton().CacheCameraSettings(TheRnd.GetWorldCamCopy());
}

void NgPostProc::Terminate() { ReleaseTex(); }

void NgPostProc::SetBloomColor() {
    const float max = Max(1.0f, mBloomThreshold);
    const float inverse = 1.0f / max;
    const Vector4 bloom(0.3f * inverse, 0.59f * inverse, 0.11f * inverse, 1.0f / inverse);
    TheShaderMgr.SetPConstant((PShaderConstant)7, bloom);
}

void NgPostProc::CheckGradientMap() {
    if (DoGradientMap()) {
        Vector4 vec(
            mGradientMapStart, mGradientMapEnd, mGradientMapIndex, mGradientMapOpacity
        );
        TheShaderMgr.SetPConstant((PShaderConstant)3, mGradientMap.Ptr());
        TheShaderMgr.SetPConstant((PShaderConstant)118, vec);
        TheShaderMgr.SetUnk3a(mGradientMap.Ptr());
    }
}

void NgPostProc::CheckVignette() {
    if (DoVignette()) {
        Vector4 vec(
            mVignetteColor.red,
            mVignetteColor.green,
            mVignetteColor.blue,
            mVignetteIntensity
        );
        TheShaderMgr.SetPConstant((PShaderConstant)123, vec);
        TheShaderMgr.SetUnk3e(true);
    }
}

void NgPostProc::CheckMotionBlur() {
    if (DoMotionBlur()) {
        float blend = mMotionBlurBlend;
        Vector4 vec(
            mMotionBlurWeight.red * blend,
            mMotionBlurWeight.green * blend,
            mMotionBlurWeight.blue * blend,
            mMotionBlurWeight.alpha
        );
        TheShaderMgr.SetPConstant((PShaderConstant)105, vec);
        TheShaderMgr.SetUnk38(true);
    }
}

void NgPostProc::CheckBlendPrevious() {
    Vector4 vec(mBlendVec.x, mBlendVec.y, mBlendVec.z, 0.0f);
    TheShaderMgr.SetPConstant((PShaderConstant)125, vec);
}
void NgPostProc::DoVelocity() {
    TheShaderMgr.SetUnk39(false);
    if (mMotionBlurVelocity != false && TheHiResScreen.IsActive() == false) {
        bool flag = RndVelocityBuffer::Singleton().Draw(TheRnd.GetWorldCamCopy(), unk23c);
        if (flag) {
            TheShaderMgr.SetUnk39(true);
            Vector4 vec(
                RndVelocityBuffer::Singleton().GetUnk36be8(),
                RndVelocityBuffer::Singleton().GetUnk36be8(),
                RndVelocityBuffer::Singleton().GetUnk36be8(),
                RndVelocityBuffer::Singleton().GetUnk36be8()
            );
            TheShaderMgr.SetPConstant((PShaderConstant)122, vec);
        }
    }
    if (!unk23c.empty())
        unk23c.clear();
}

void NgPostProc::CheckNoise() {
    bool flag = mNoiseIntensity && mNoiseMap;
    if (flag) {
        if (!mNoiseStationary) {
            Vector4 v80(RandomFloat(), RandomFloat(), RandomFloat(), RandomFloat());
            TheShaderMgr.SetPConstant((PShaderConstant)0x70, v80);
            Vector4 v70(
                mNoiseBaseScale.x, mNoiseBaseScale.y, mNoiseTopScale, mNoiseIntensity
            );
            TheShaderMgr.SetPConstant((PShaderConstant)0x71, v70);
        } else {
            Vector4 v60(unk22c.x, unk22c.y, unk22c.x, unk22c.y);

            // Vector4 v60(unk22c, unk230, unk22c, unk230);
            TheShaderMgr.SetPConstant((PShaderConstant)0x70, v60);
            Vector4 v50(mNoiseBaseScale.x, mNoiseBaseScale.y, 1, mNoiseIntensity);
            TheShaderMgr.SetPConstant((PShaderConstant)0x71, v50);
        }
        TheShaderMgr.SetPConstant((PShaderConstant)0xD, mNoiseMap);
        TheRenderState.SetTextureFilter(13, RndRenderState::kFilterModeLinear, false);
        TheRenderState.SetTextureClamp(13, RndRenderState::kClampModeWrap);
    }
    TheShaderMgr.SetUnk2d(flag);
    TheShaderMgr.SetUnk2e(flag ? mNoiseMidtone : false);
}

void NgPostProc::CheckHallOfTime() {
    if (HallOfTime() && !unk250) {
        Vector4 c1(mHallOfTimeRate, mHallOfTimeMix, 0.0f, 0.0f);
        TheShaderMgr.SetPConstant((PShaderConstant)0x73, c1);

        Vector4 c2(
            mHallOfTimeColor.red, mHallOfTimeColor.green, mHallOfTimeColor.blue, 1.0f
        );
        TheShaderMgr.SetPConstant((PShaderConstant)0x74, c2);

        TheShaderMgr.SetUnk34(mHallOfTimeType + 1);
    }
}

void NgPostProc::DoPost() {
    RndPostProc::DoPost();
    DoVelocity();
    DoBloom();
    ModulateColorXfm();
    CheckNoise();
    CheckBlendPrevious();
    CheckHallOfTime();
    CheckMotionBlur();
    CheckGradientMap();
    CheckHueConverge();
    CheckRefract();
    CheckChromaticAberration();
    CheckPosterizeAndKaleidoscope();
    CheckVignette();
    TheShaderMgr.SetUnk29(ColorXfmEnabled());
    TheShaderMgr.SetUnk2f(BlendPrevious());
    unk250 = false;
}

void NgPostProc::QueueMotionBlurObject(RndDrawable *drawable) {
    if (unk23c.find(drawable) == unk23c.end()) {
        unk23c.push_back(drawable);
    }
}

void NgPostProc::CheckPosterizeAndKaleidoscope() {
    Vector4 v30(0, 0, 0, 0);
    Vector4 v20(0, 0, 0, 0);
    if (mPosterLevels * mPosterMin > 0) {
        TheShaderMgr.SetUnk2b(true);
        v30.x = mPosterLevels;
        v30.y = mPosterLevels * mPosterMin;
    }
    if (mKaleidoscopeComplexity > 0) {
        TheShaderMgr.SetUnk2c(true);
        v30.z = (2 * PI) / mKaleidoscopeComplexity;
        v30.w = mKaleidoscopeSize;
        v20.x = mKaleidoscopeAngle * DEG2RAD;
        v20.y = mKaleidoscopeRadius;
        if (mKaleidoscopeFlipUVs) {
            v20.z = 2;
        } else {
            v20.z = 1;
        }
    }
    TheShaderMgr.SetPConstant((PShaderConstant)8, v30);
    TheShaderMgr.SetPConstant((PShaderConstant)0x75, v20);
}

void NgPostProc::CheckHueConverge() {
    if (DoHueConverge()) {
        Vector4 v30(
            mHueTarget * 0.0027777778f + 0.5f, mHueFocus, mBlendAmount, mBrightnessPower
        );
        TheShaderMgr.SetPConstant((PShaderConstant)0xDE, v30);
        TheShaderMgr.SetUnk2a(mBlendAmount > 0);
    }
}

void NgPostProc::CheckRefract() {
    if (DoRefraction()) {
        float x = mRefractAngle * DEG2RAD;
        float sinx = sinf(x);
        float cosx = cosf(x);
        Vector4 v50(x, sinx, cosx, mRefractDist);
        unk234.x += mRefractVelocity.x * mDeltaSecs;
        unk234.y += mRefractVelocity.y * mDeltaSecs;
        unk234.x = fmodf(unk234.x, 1);
        unk234.y = fmodf(unk234.y, 1);
        Vector4 v40(
            mRefractScale.x,
            mRefractScale.y,
            unk234.x + mRefractPanning.x,
            mRefractPanning.y + unk234.y
        );
        TheShaderMgr.SetPConstant((PShaderConstant)0x77, v50);
        TheShaderMgr.SetPConstant((PShaderConstant)0x78, v40);
        TheShaderMgr.SetPConstant((PShaderConstant)1, mRefractMap);
        TheRenderState.SetTextureFilter(1, RndRenderState::kFilterModeLinear, false);
        TheRenderState.SetTextureClamp(1, RndRenderState::kClampModeWrap);
        TheShaderMgr.SetUnk3b(true);
    } else {
        unk234.x = 0;
        unk234.y = 0;
    }
}

void NgPostProc::CheckChromaticAberration() {
    if (DoChromaticAberration()) {
        Vector4 v30(
            mChromaticAberrationOffset / (float)TheRnd.Width(),
            mChromaticAberrationOffset / (float)TheRnd.Height(),
            0,
            0
        );
        TheShaderMgr.SetPConstant((PShaderConstant)0x79, v30);
        if (mChromaticSharpen) {
            TheShaderMgr.SetUnk3d(true);
        } else {
            TheShaderMgr.SetUnk3c(true);
        }
    }
}

void NgPostProc::ModulateColorXfm() {
    float scale = mColorModulation;
    Transform colorXfm = mColorXfm.mColorXfm;
    if (scale != 1) {
        Scale(colorXfm.m.x, scale, colorXfm.m.x);
        Scale(colorXfm.m.y, scale, colorXfm.m.y);
        Scale(colorXfm.m.z, scale, colorXfm.m.z);
    }
    TheShaderMgr.SetPConstant4x3((PShaderConstant)0x5C, Hmx::Matrix4(colorXfm));
}

void Bloom_Downsample(ShaderType shader, RndTex *texSrc, RndTex *texDst) {
    MILO_ASSERT((shader == kBloomShader) || (shader == kDownsample4xShader), 0x17B);
    MILO_ASSERT(texDst->Width() > 0, 0x17C);
    MILO_ASSERT(texDst->Height() > 0, 0x17D);
    MILO_ASSERT(texDst->Width() < texSrc->Width(), 0x17E);
    MILO_ASSERT(texDst->Height() < texSrc->Height(), 0x17F);

    RndMat *work = TheShaderMgr.GetWork();
    work->SetBlend(BaseMaterial::kBlendSrc);
    work->SetZMode(kZModeDisable);
    work->SetTexWrap(kTexWrapClamp);
    texDst->MakeDrawTarget();
    TheShaderMgr.SetPConstant((PShaderConstant)7, texSrc);
    work->SetDiffuseTex(texSrc);

    Hmx::Rect r(0, 0, texDst->Width(), texDst->Height());
    Hmx::Color c;
    TheNgRnd.DrawRect(r, work, shader, c, nullptr, nullptr);

    texDst->FinishDrawTarget();
}

void Bloom_Blur(
    RndTex *texSrc,
    RndTex *texDst,
    BloomBlurStyle bloomStyle,
    BloomBlurDirection bloomDirection,
    unsigned int ui5,
    float f6,
    float f7
) {
    MILO_ASSERT(texDst->Width() > 0, 0x1B2);
    MILO_ASSERT(texDst->Height() > 0, 0x1B3);
    MILO_ASSERT(texDst->Width() == texSrc->Width(), 0x1B4);
    MILO_ASSERT(texDst->Height() == texSrc->Height(), 0x1B5);
    MILO_ASSERT(TheShaderMgr.NumTaps() == 1, 0x1B6);

    RndMat *work = TheShaderMgr.GetWork();
    work->SetZMode(kZModeDisable);
    work->SetTexWrap(kTexWrapClamp);
    work->SetBlend(BaseMaterial::kBlendSrc);
    texDst->MakeDrawTarget();
    work->SetDiffuseTex(texSrc);
    ShaderType shader = kBlurShader;
    bool b1 = bloomDirection == 0;
    float w = texDst->Width();
    float h = texDst->Height();
    switch (bloomStyle) {
    case kBloomBlurStyle0:
        SetBloomBlurWeights(b1, w, h);
        break;
    case kBloomBlurStyle1:
        SetBloomBlurWeightsStreak(b1, w, h, f6, ui5, f7);
        break;
    case kBloomBlurStyle2:
        shader = kBloomGlareShader;
        break;
    }
    Hmx::Rect r(0, 0, texDst->Width(), texDst->Height());
    Hmx::Color c;
    TheNgRnd.DrawRect(r, work, shader, c, nullptr, nullptr);
    TheShaderMgr.SetNumTaps(1);
    texDst->FinishDrawTarget();
}

void NgPostProc::DoBloom() {
    bool b1, b2;

    if (BloomIntensity() > 0 || mBloomColor.alpha > 0) {
        b2 = true;
    } else {
        b2 = false;
    }
    if (mBloomGlare && !TheHiResScreen.IsActive()) {
        b1 = true;
    } else {
        b1 = false;
    }

    if (!b2 && s_BloomSetter) {
        RndOverlay *ppOverlay = RndOverlay::Find("postproc");
        if (ppOverlay->Showing()) {
            TextStream *r = TheDebug.Reflect();
            TheDebug.SetReflect(ppOverlay);
            MILO_LOG("BLOOM : NONE\n");
            TheDebug.SetReflect(r);
        }
        s_BloomSetter = nullptr;
    }

    if (b2) {
        float scalar = BloomIntensity() * sBloomLocFactor;
        Vector4 vc0(
            mBloomColor.red * scalar,
            mBloomColor.green * scalar,
            mBloomColor.blue * scalar,
            0
        );
        if (mBloomColor != s_prevBloomColor || BloomIntensity() != s_prevBloomIntensity
            || s_BloomSetter != this) {
            s_prevBloomColor = mBloomColor;
            s_prevBloomIntensity = BloomIntensity();
            s_BloomSetter = this;
            RndOverlay *ppOverlay = RndOverlay::Find("postproc");
            if (ppOverlay->Showing()) {
                TextStream *r = TheDebug.Reflect();
                RndPostProc *path = TheHamDirector->GetUnk18c();
                TheDebug.SetReflect(ppOverlay);
                static int s8a30;
                MILO_LOG(
                    "%03d:BLOOM: C=<%3d,%3d,%3d> I=%5.2f : %s\n",
                    s8a30++ % 100,
                    (int)(mBloomColor.red * 256.0),
                    (int)(mBloomColor.green * 256.0),
                    (int)(mBloomColor.blue * 256.0),
                    BloomIntensity(),
                    PathName(path)
                );
                TheDebug.SetReflect(r);
            }
        }
        TheShaderMgr.SetPConstant((PShaderConstant)6, vc0);
        TheShaderMgr.SetPConstant(
            (PShaderConstant)7, TheRnd.GetDefaultTex(Rnd::kDefaultTex_Black)
        );
        TheShaderMgr.SetPConstant(
            (PShaderConstant)11, TheRnd.GetDefaultTex(Rnd::kDefaultTex_Black)
        );
        TheShaderMgr.SetPConstant(
            (PShaderConstant)15, TheRnd.GetDefaultTex(Rnd::kDefaultTex_Black)
        );
        TheRenderState.SetTextureFilter(7, RndRenderState::kFilterModeLinear, false);
        TheRenderState.SetTextureClamp(7, RndRenderState::kClampModeClamp);
        TheRenderState.SetTextureFilter(7, RndRenderState::kFilterModeLinear, false);
        TheRenderState.SetTextureClamp(7, RndRenderState::kClampModeClamp);
        TheRenderState.SetTextureFilter(11, RndRenderState::kFilterModeLinear, false);
        TheRenderState.SetTextureClamp(11, RndRenderState::kClampModeClamp);
        TheRenderState.SetTextureFilter(15, RndRenderState::kFilterModeLinear, false);
        TheRenderState.SetTextureClamp(15, RndRenderState::kClampModeClamp);
        RndTex *preProcessTex = TheNgRnd.PreProcessTexture();
        if (preProcessTex) {
            if (b1) {
                Bloom_Downsample(kBloomShader, preProcessTex, sBloom.Tex(0, 1));
                // clang-format off
                Bloom_Blur(sBloom.Tex(0, 1), sBloom.Tex(0, 0), kBloomBlurStyle0, kBloomBlurDirection0, 0, 0, 0);
                Bloom_Blur(sBloom.Tex(0, 0), sBloom.Tex(0, 1), kBloomBlurStyle0, kBloomBlurDirection1, 0, 0, 0);
                Bloom_Blur(sBloom.Tex(0, 1), sBloom.Tex(0, 0), kBloomBlurStyle2, kBloomBlurDirection0, 0, 0, 0);
                // clang-format on
                TheShaderMgr.SetPConstant((PShaderConstant)7, sBloom.Tex(0, 0));
            } else if (mBloomStreak && !mBloomGlare) {
                Bloom_Downsample(kBloomShader, preProcessTex, sBloom.Tex(0, 1));
                // clang-format off
                Bloom_Blur(sBloom.Tex(0, 1), sBloom.Tex(0, 0), kBloomBlurStyle1, kBloomBlurDirection0, 0, mBloomStreakAttenuation, mBloomStreakAngle);
                Bloom_Blur(sBloom.Tex(0, 0), sBloom.Tex(0, 1), kBloomBlurStyle1, kBloomBlurDirection0, 1, mBloomStreakAttenuation, mBloomStreakAngle);
                Bloom_Blur(sBloom.Tex(0, 1), sBloom.Tex(0, 0), kBloomBlurStyle1, kBloomBlurDirection0, 2, mBloomStreakAttenuation, mBloomStreakAngle);
                Bloom_Downsample(kBloomShader, preProcessTex, sBloom.Tex(1, 1));
                Bloom_Blur(sBloom.Tex(1, 1), sBloom.Tex(1, 0), kBloomBlurStyle1, kBloomBlurDirection1, 0, mBloomStreakAttenuation, mBloomStreakAngle);
                Bloom_Blur(sBloom.Tex(1, 0), sBloom.Tex(1, 1), kBloomBlurStyle1, kBloomBlurDirection1, 1, mBloomStreakAttenuation, mBloomStreakAngle);
                Bloom_Blur(sBloom.Tex(1, 1), sBloom.Tex(1, 0), kBloomBlurStyle1, kBloomBlurDirection1, 2, mBloomStreakAttenuation, mBloomStreakAngle);
                // clang-format on
                TheShaderMgr.SetPConstant((PShaderConstant)7, sBloom.Tex(0, 0));
                TheShaderMgr.SetPConstant((PShaderConstant)11, sBloom.Tex(1, 0));
            } else {
                Bloom_Downsample(kBloomShader, preProcessTex, sBloom.Tex(0, 0));
                // clang-format off
                Bloom_Blur(sBloom.Tex(0, 0), sBloom.Tex(0, 1), kBloomBlurStyle0, kBloomBlurDirection0, 0, 0, 0);
                Bloom_Blur(sBloom.Tex(0, 1), sBloom.Tex(0, 0), kBloomBlurStyle0, kBloomBlurDirection1, 0, 0, 0);
                Bloom_Downsample(kDownsample4xShader, sBloom.Tex(0, 0), sBloom.Tex(1, 0));
                Bloom_Blur(sBloom.Tex(1, 0), sBloom.Tex(1, 1), kBloomBlurStyle0, kBloomBlurDirection0, 0, 0, 0);
                Bloom_Blur(sBloom.Tex(1, 1), sBloom.Tex(1, 0), kBloomBlurStyle0, kBloomBlurDirection1, 0, 0, 0);
                Bloom_Downsample(kDownsample4xShader, sBloom.Tex(1, 0), sBloom.Tex(2, 0));
                Bloom_Blur(sBloom.Tex(2, 0), sBloom.Tex(2, 1), kBloomBlurStyle0, kBloomBlurDirection0, 0, 0, 0);
                Bloom_Blur(sBloom.Tex(2, 1), sBloom.Tex(2, 0), kBloomBlurStyle0, kBloomBlurDirection1, 0, 0, 0);
                // clang-format on
                TheShaderMgr.SetPConstant((PShaderConstant)7, sBloom.Tex(0, 0));
                TheShaderMgr.SetPConstant((PShaderConstant)11, sBloom.Tex(1, 0));
                TheShaderMgr.SetPConstant((PShaderConstant)15, sBloom.Tex(2, 0));
            }
        }
    } else {
        s_BloomSetter = nullptr;
        s_prevBloomIntensity = -1;
        s_prevBloomColor = Hmx::Color(-1, -1, -1, -1);
    }

    if (b2) {
        if (b1) {
            TheShaderMgr.SetUnk28(true);
            TheShaderMgr.SetUnk27(false);
        } else if (b2) {
            TheShaderMgr.SetUnk28(false);
            TheShaderMgr.SetUnk27(true);
        }
    } else {
        TheShaderMgr.SetUnk27(false);
        TheShaderMgr.SetUnk28(false);
    }
}
