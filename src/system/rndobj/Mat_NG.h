#pragma once
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rndobj/Mat.h"
#include "rndobj/RenderState.h"

// how a mat fades out with the environment's fade out, picked from whatever blend mode it
// is using
enum FadeType {
    // kBlendSrc, there is nothing to fade
    kFade_None = 0,
    // alpha blends, fades alpha to 0
    kFade_Alpha = 1,
    // add/multiply style blends, fades color to whatever makes the blend do nothing?
    kFade_Color = 2,
    kFade_Count = 3
};

class NgMat : public RndMat {
public:
    NgMat();
    virtual ~NgMat();
    OBJ_CLASSNAME(Mat);
    OBJ_SET_TYPE(Mat);

    bool AllowFog() const;
    bool AllowHDR() const;
    const Hmx::Color &FadeColor() const { return mFadeColor; }
    FadeType GetFadeType() const { return mFadeType; }
    void SetupShader(bool, bool);

    NEW_OBJ(NgMat);
    static NgMat *Current() { return sCurrent; }
    static void SetCurrent(NgMat *c) { sCurrent = c; }

protected:
    static NgMat *sCurrent;

    void SetupAmbient();
    void SetBasicState();
    void RefreshState();
    void SetRegularShaderConst(bool);

    Vector4 mTexelOffsets;
    RndRenderState::Blend mBlendA; // 0x23c
    RndRenderState::Blend mBlendB; // 0x240
    bool mDepthTestEnable; // 0x244
    bool mDepthWriteEnable; // 0x245
    RndRenderState::TestFunc mDepthFunc; // 0x248
    RndRenderState::TestFunc mStencilFunc; // 0x24c
    RndRenderState::StencilOp mStencilPass; // 0x250
    Hmx::Matrix4 mCachedXfm;
    Hmx::Matrix4 mCachedXfmNorm;
    FadeType mFadeType; // 0x2d4
    Hmx::Color mFadeColor; // 0x2d8
    RndRenderState::BlendOp mBlendOp; // 0x2e8
    bool mBlendEnable; // 0x2ec
};
