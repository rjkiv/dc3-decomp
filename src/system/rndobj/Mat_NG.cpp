#include "rndobj/Mat_NG.h"
#include "BaseMaterial.h"
#include "ShaderMgr.h"
#include "math/Color.h"
#include "math/Mtx.h"
#include "math/Rot.h"
#include "math/Trig.h"
#include "math/Utl.h"
#include "os/Debug.h"
#include "rndobj/Cam.h"
#include "rndobj/HiResScreen.h"
#include "rndobj/RenderState.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Env.h"
#include "rndobj/Rnd.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/Utl.h"

NgMat *NgMat::sCurrent;

RndRenderState::ClampMode sTexWrapClampModes[6] = {
    RndRenderState::kClampModeClamp,  RndRenderState::kClampModeWrap,
    RndRenderState::kClampModeBorder, RndRenderState::kClampModeBorder,
    RndRenderState::kClampModeMirror, RndRenderState::kClampModeWrap
};

void MakeTex3(const Transform &xfm, bool b2, Hmx::Matrix4 &mtx) {
    mtx.m[0].Set(xfm.m.x.x, -xfm.m.x.y, 0, 0);
    mtx.m[1].Set(-xfm.m.y.x, xfm.m.y.y, 0, 0);
    mtx.m[2].Set(0, 0, 1, 0);
    if (b2) {
        float vx = -xfm.v.x - 0.5f;
        float vy = xfm.v.y - 0.5f;
        mtx.m[3].z = 0;
        mtx.m[3].w = 1;
        mtx.m[3].y = mtx.m[0].y * vx + mtx.m[1].y * vy + 0.5f;
        mtx.m[3].x = mtx.m[1].x * vy + mtx.m[0].x * vx + 0.5f;
    } else {
        mtx.m[3].Set(xfm.v.x, xfm.v.y, 0, 1);
    }
}

NgMat::NgMat() {}

NgMat::~NgMat() {
    if (sCurrent == this)
        sCurrent = nullptr;
}

bool NgMat::AllowFog() const {
    return mBlend != kBlendDest && mBlend != kBlendAdd && mBlend != kBlendSubtract
        && mBlend != kBlendSrcAlphaAdd;
}

bool NgMat::AllowHDR() const {
    return (mBlend != kBlendSrcAlpha && mBlend != kBlendSrcAlphaAdd
            && mBlend != kPreMultAlpha)
        && !mAlphaCut && !mAlphaWrite;
}

void NgMat::SetupShader(bool b1, bool b2) {
    if (b2) {
        SetupAmbient();
    }
    if (this != sCurrent || mDirty) {
        if (mDirty & 2) {
            RefreshState();
        }
        SetBasicState();
        if (b2) {
            SetRegularShaderConst(b1);
        }
        mDirty = 0;
        sCurrent = this;
    }
}

void NgMat::SetBasicState() {
    RndRenderState::CullMode cm;
    switch (mCull) {
    case kCullNone:
        cm = RndRenderState::kCullModeNone;
        break;
    case kCullRegular:
        cm = RndRenderState::kCullModeCW;
        break;
    case kCullBackwards:
        cm = RndRenderState::kCullModeCCW;
        break;
    default:
        cm = RndRenderState::kCullModeCW;
        break;
    }
    TheRenderState.SetCullMode(cm);
    TheRenderState.SetBlendEnable(mBlendEnable);
    TheRenderState.SetBlendOp(mBlendOp);
    TheRenderState.SetBlend(mBlendA, mBlendB, mBlendA, mBlendB);
    TheRenderState.SetAlphaTestEnable(mAlphaCut);
    if (mAlphaCut) {
        TheRenderState.SetAlphaFunc(RndRenderState::kTestFuncGreater, mAlphaThreshold);
    }
    TheRenderState.SetDepthTestEnable(mDepthTestEnable);
    TheRenderState.SetDepthWriteEnable(mDepthWriteEnable);
    TheRenderState.SetDepthFunc(mDepthFunc);
    if (mStencilMode == kStencilIgnore) {
        TheRenderState.SetStencilTestEnable(false);
    } else {
        TheRenderState.SetStencilTestEnable(true);
        TheRenderState.SetStencilFunc(mStencilFunc, 0);
        TheRenderState.SetStencilOp(
            RndRenderState::kStencilOpKeep, RndRenderState::kStencilOpKeep, mStencilPass
        );
    }
    RndRenderState::ClampMode cur = sTexWrapClampModes[mTexWrap];
    if (mDiffuseTex) {
        TheRenderState.SetTextureClamp(0, cur);
    }
    if (mDiffuseTex2) {
        TheRenderState.SetTextureClamp(6, cur);
    }
    if (mNormalMap) {
        TheRenderState.SetTextureClamp(1, cur);
    }
    if (mSpecularMap) {
        TheRenderState.SetTextureClamp(2, cur);
    }
    if (mEmissiveMap) {
        TheRenderState.SetTextureClamp(3, cur);
    }
    if (cur == 6) {
        bool white = mTexWrap == kTexBorderWhite;
        TheRenderState.SetBorderColor(0, white);
        TheRenderState.SetBorderColor(6, white);
        TheRenderState.SetBorderColor(1, white);
        TheRenderState.SetBorderColor(2, white);
        TheRenderState.SetBorderColor(3, white);
    }
}

void NgMat::SetupAmbient() {
    Hmx::Color color;
    if (mUseEnviron) {
        color = RndEnviron::Current()->AmbientColor();
    } else {
        color.Set(1, 1, 1, 1);
    }
    TheShaderMgr.SetVConstant(
        kVShader_AmbientCol, Vector4(color.red, color.green, color.blue, color.alpha)
    );
    TheShaderMgr.SetPConstant(
        kPShader_AmbientCol, Vector4(color.red, color.green, color.blue, color.alpha)
    );
}

void NgMat::SetRegularShaderConst(bool b1) {
    if (mEmissiveMap || mIntensify || AllowHDR()) {
        TheShaderMgr.SetPConstant(
            (PShaderConstant)5,
            Vector4(mEmissiveMultiplier, (float)(mIntensify + 1), mBloomMultiplier, 0)
        );
    }
    if (mNormalMap) {
        TheShaderMgr.SetPConstant((PShaderConstant)1, mNormalMap);
        float val = 1 - mDeNormal;
        TheShaderMgr.SetPConstant((PShaderConstant)0xE, Vector4(val, val, val, val));
        TheRenderState.SetTextureFilter(1, RndRenderState::kFilterModeLinear, false);
    }
    if (mNormDetailMap) {
        TheShaderMgr.SetPConstant((PShaderConstant)0xE, mNormDetailMap);
        TheRenderState.SetTextureFilter(0xE, RndRenderState::kFilterModeLinear, false);
        TheRenderState.SetTextureClamp(0xE, RndRenderState::kClampModeWrap);
        TheShaderMgr.SetPConstant(
            (PShaderConstant)0x6A, Vector4(mNormDetailStrength, mNormDetailTiling, 1, 1)
        );
    }
    if (mEmissiveMap) {
        TheShaderMgr.SetPConstant((PShaderConstant)3, mEmissiveMap);
    }
    Hmx::Color color = mColor;
    if (mBlend == kPreMultAlpha) {
        PreMultiplyAlpha(color);
    }
    TheShaderMgr.SetVConstant(
        (VShaderConstant)0, Vector4(color.red, color.green, color.blue, color.alpha)
    );
    TheShaderMgr.SetPConstant(
        (PShaderConstant)0, Vector4(color.red, color.green, color.blue, color.alpha)
    );
    TheShaderMgr.SetPConstant((PShaderConstant)0, mDiffuseTex);
    TheShaderMgr.SetPConstant((PShaderConstant)6, mDiffuseTex2);
    TheRenderState.SetTextureFilter(
        0, RndRenderState::kFilterModeLinear, mPerfSettings.mPS3ForceTrilinear
    );
    TheRenderState.SetTextureFilter(
        6, RndRenderState::kFilterModeLinear, mPerfSettings.mPS3ForceTrilinear
    );
    TheShaderMgr.SetPConstant((PShaderConstant)0xF, mTexelOffsets);

    Hmx::Color specularColor = mSpecularRGB;
    Hmx::Color specularColor2 = mSpecular2RGB;
    MaxEq(specularColor.alpha, 0.5f);
    MaxEq(specularColor2.alpha, 0.5f);

    if (!b1 && mPerPixelLit && mSpecularMap) {
        Multiply(specularColor, 0.4f, specularColor);
        Multiply(specularColor2, 0.4f, specularColor2);
    }

    TheShaderMgr.SetVConstant(
        (VShaderConstant)2,
        Vector4(
            specularColor.red, specularColor.green, specularColor.blue, specularColor.alpha
        )
    );
    TheShaderMgr.SetPConstant(
        (PShaderConstant)2,
        Vector4(
            specularColor.red, specularColor.green, specularColor.blue, specularColor.alpha
        )
    );
    if (mSpecularMap) {
        TheShaderMgr.SetPConstant((PShaderConstant)2, mSpecularMap);
        TheRenderState.SetTextureFilter(2, RndRenderState::kFilterModeLinear, false);
    }
    Hmx::Color rimColor = mRimRGB;
    rimColor.alpha = Max(rimColor.alpha, 0.5f);
    TheShaderMgr.SetVConstant(
        (VShaderConstant)0x3D,
        Vector4(rimColor.red, rimColor.green, rimColor.blue, rimColor.alpha)
    );
    TheShaderMgr.SetPConstant(
        (PShaderConstant)0x3D,
        Vector4(rimColor.red, rimColor.green, rimColor.blue, rimColor.alpha)
    );
    if (mRimMap) {
        TheShaderMgr.SetPConstant((PShaderConstant)0xF, mRimMap);
        TheRenderState.SetTextureFilter(0xF, RndRenderState::kFilterModeLinear, false);
    }
    if (mFlags != kColorModNone) {
        for (int i = 0; i < kColorModNum; i++) {
            TheShaderMgr.SetPConstant(
                (PShaderConstant)(0x83 + i),
                Vector4(
                    mColorMod[i].red,
                    mColorMod[i].green,
                    mColorMod[i].blue,
                    mColorMod[i].alpha
                )
            );
        }
    }
    if (mEnvironMap) {
        TheShaderMgr.SetPConstant((PShaderConstant)4, mEnvironMap);
        TheRenderState.SetTextureFilter(4, RndRenderState::kFilterModeLinear, false);
    }
    if (mShaderVariation != kShaderVariationNone) {
        TheShaderMgr.SetVConstant(
            (VShaderConstant)0x13,
            Vector4(
                specularColor2.red,
                specularColor2.green,
                specularColor2.blue,
                specularColor2.alpha
            )
        );
        TheShaderMgr.SetPConstant(
            (PShaderConstant)0x13,
            Vector4(
                specularColor2.red,
                specularColor2.green,
                specularColor2.blue,
                specularColor2.alpha
            )
        );
        float y = 1 / (mWorldProjectionEndBlend - mWorldProjectionStartBlend);
        TheShaderMgr.SetPConstant(
            (PShaderConstant)0xDC,
            Vector4(mWorldProjectionTiling, y, -(mWorldProjectionStartBlend * y), 0)
        );
    }
    TheShaderMgr.SetVConstant((VShaderConstant)0x14, mCachedXfm);
    if (mAnisotropy > 0) {
        TheShaderMgr.SetPConstant((PShaderConstant)0xD, Vector4(mAnisotropy, 0, 0, 1));
    }
    if (GetRefractEnabled(false)) {
        RndTex *refractMap = GetRefractNormalMap();
        RndTex *frameTex = TheRnd.GetCurrentFrameTex(TheHiResScreen.IsActive());
        if (refractMap && frameTex) {
            TheShaderMgr.SetPConstant((PShaderConstant)1, refractMap);
            TheRenderState.SetTextureFilter(1, RndRenderState::kFilterModeLinear, false);
            TheRenderState.SetTextureClamp(1, sTexWrapClampModes[mTexWrap]);
            TheShaderMgr.SetPConstant((PShaderConstant)6, frameTex);
            TheRenderState.SetTextureFilter(6, RndRenderState::kFilterModeLinear, false);
            TheRenderState.SetTextureClamp(6, RndRenderState::kClampModeClamp);
            float strength = GetRefractStrength();
            TheShaderMgr.SetPConstant(
                (PShaderConstant)0x77, Vector4(strength, strength, strength, strength)
            );
        }
    }
}

void NgMat::RefreshState() {
    if (mDiffuseTex && mDiffuseTex->Width() && mDiffuseTex->Height()) {
        mTexelOffsets.Set(
            (float)mDiffuseTex->Width() / 2,
            (float)mDiffuseTex->Height() / 2,
            -(float)mDiffuseTex->Width() / 2,
            -(float)mDiffuseTex->Height() / 2
        );
    } else {
        mTexelOffsets.Set(0, 0, 0, 0);
    }
    switch (mBlend) {
    case kBlendDest:
        mBlendA = RndRenderState::kBlendZero;
        mBlendOp = RndRenderState::kBlendOpAdd;
        mBlendB = RndRenderState::kBlendOne;
        mBlendEnable = true;
        break;
    case kBlendSrc:
        mBlendA = RndRenderState::kBlendOne;
        mBlendB = RndRenderState::kBlendZero;
        mBlendOp = RndRenderState::kBlendOpAdd;
        mBlendEnable = false;
        break;
    case kBlendAdd:
        mBlendOp = RndRenderState::kBlendOpAdd;
        mBlendA = RndRenderState::kBlendOne;
        mBlendB = RndRenderState::kBlendOne;
        mBlendEnable = true;
        break;
    case kBlendSrcAlpha:
        mBlendOp = RndRenderState::kBlendOpAdd;
        mBlendA = RndRenderState::kBlendSrcAlpha;
        mBlendB = RndRenderState::kBlendInvSrcAlpha;
        mBlendEnable = true;
        break;
    case kBlendSrcAlphaAdd:
        mBlendOp = RndRenderState::kBlendOpAdd;
        mBlendA = RndRenderState::kBlendSrcAlpha;
        mBlendB = RndRenderState::kBlendOne;
        mBlendEnable = true;
        break;
    case kBlendSubtract:
        mBlendOp = RndRenderState::kBlendOpRevSubtract;
        mBlendA = RndRenderState::kBlendOne;
        mBlendB = RndRenderState::kBlendOne;
        mBlendEnable = true;
        break;
    case kBlendMultiply:
        mBlendA = RndRenderState::kBlendZero;
        mBlendB = RndRenderState::kBlendSrcColor;
        mBlendOp = RndRenderState::kBlendOpAdd;
        mBlendEnable = true;
        break;
    case kPreMultAlpha:
        mBlendA = RndRenderState::kBlendOne;
        mBlendOp = RndRenderState::kBlendOpAdd;
        mBlendB = RndRenderState::kBlendInvSrcAlpha;
        mBlendEnable = true;
        break;
    case kScreen:
        mBlendOp = RndRenderState::kBlendOpAdd;
        mBlendA = RndRenderState::kBlendInvDestColor;
        mBlendB = RndRenderState::kBlendOne;
        mBlendEnable = true;
        break;
    case kLighten:
        mBlendOp = RndRenderState::kBlendOpMax;
        mBlendA = RndRenderState::kBlendOne;
        mBlendB = RndRenderState::kBlendOne;
        mBlendEnable = true;
        break;
    case kDarken:
        mBlendOp = RndRenderState::kBlendOpMin;
        mBlendA = RndRenderState::kBlendOne;
        mBlendB = RndRenderState::kBlendOne;
        mBlendEnable = true;
        break;
    default:
        break;
    }
    switch (mZMode) {
    case kZModeDisable:
        mDepthTestEnable = false;
        mDepthFunc = RndRenderState::kTestFuncLess;
        mDepthWriteEnable = false;
        break;
    case kZModeNormal:
        mDepthTestEnable = true;
        mDepthWriteEnable = true;
        mDepthFunc = RndRenderState::kTestFuncLess;
        break;
    case kZModeTransparent:
        mDepthTestEnable = true;
        mDepthFunc = RndRenderState::kTestFuncLessEqual;
        mDepthWriteEnable = false;
        break;
    case kZModeForce:
        mDepthTestEnable = true;
        mDepthWriteEnable = true;
        mDepthFunc = RndRenderState::kTestFuncAlways;
        break;
    case kZModeDecal:
        mDepthTestEnable = true;
        mDepthWriteEnable = true;
        mDepthFunc = RndRenderState::kTestFuncLessEqual;
        break;
    default:
        break;
    }
    if (mStencilMode == kStencilWrite) {
        mStencilFunc = RndRenderState::kTestFuncAlways;
        mStencilPass = RndRenderState::kStencilOpIncrSat;
    } else {
        mStencilFunc = RndRenderState::kTestFuncNotEqual;
        mStencilPass = RndRenderState::kStencilOpKeep;
    }
    static Transform sStateXfm(Hmx::Matrix3(1, 0, 0, 0, 0, 1, 0, 1, 0), Vector3(0, 0, 0));
    mCachedXfmNorm.Identity();
    Transform tf160;
    switch (mTexGen) {
    case kTexGenNone:
        mCachedXfm.Zero();
        mCachedXfm.m[3].w = 1;
        mCachedXfm.m[2].z = 1;
        mCachedXfm.m[1].y = 1;
        mCachedXfm.m[0].x = 1;
        break;
    case kTexGenXfm:
    case kTexGenXfmOrigin: {
        MakeTex3(mTexXfm, mTexGen == kTexGenXfm, mCachedXfm);
        tf160.v.Zero();
        Normalize(mTexXfm.m, tf160.m);
        MakeTex3(tf160, mTexGen == kTexGenXfm, mCachedXfmNorm);
        break;
    }
    case kTexGenSphere:
    case kTexGenEnviron: {
        Transpose(mTexXfm.m, tf160.m);
        Multiply(RndCam::Current()->WorldXfm().m, tf160.m, tf160.m);
        if (mTexGen == kTexGenSphere) {
            Vector3 v1a0;
            MakeEuler(tf160.m, v1a0);
            v1a0.x = LimitAng(v1a0.x) / 2;
            v1a0.z = LimitAng(v1a0.z) / 2;
            MakeRotMatrix(v1a0, tf160.m, true);
        }
        Hmx::Matrix3 m190;
        Transpose(RndCam::Current()->WorldXfm().m, m190);
        Multiply(m190, tf160.m, tf160.m);
        m190.Set(0.5f, 0, 0, 0, 0, 1, 0, -0.5f, 0);
        Multiply(tf160.m, m190, tf160.m);
        tf160.v.Set(0.5f, 0.5f, 0);
        mCachedXfm = Hmx::Matrix4(tf160);
        break;
    }
    case kTexGenProjected: {
        FastInvert(mTexXfm, tf160);
        Transform tf120 = sStateXfm;
        tf120.m.z.y = -1;
        Multiply(tf160, tf120, tf160);
        mCachedXfm = Hmx::Matrix4(tf160);
        break;
    }
    default:
        break;
    }

    switch (mBlend) {
    case kBlendDest:
        break;
    case kBlendSrc:
        mFadeType = kFade_None;
        break;
    case kBlendSrcAlpha:
    case kPreMultAlpha:
        mFadeColor.Set(0, 0, 0, 0);
        mFadeType = kFade_Alpha;
        break;
    case kBlendAdd:
    case kBlendSrcAlphaAdd:
    case kBlendSubtract:
    case kScreen:
    case kLighten:
        mFadeColor.Set(0, 0, 0, 0);
        mFadeType = kFade_Color;
        break;
    case kBlendMultiply:
    case kDarken:
        mFadeColor.Set(1, 1, 1, 1);
        mFadeType = kFade_Color;
        break;
    default:
        MILO_ASSERT(false, 0x139);
        break;
    }
}
