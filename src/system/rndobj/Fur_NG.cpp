#include "Fur_NG.h"
#include "math/Color.h"
#include "math/Vec.h"
#include "rndobj/Mat_NG.h"
#include "rndobj/RenderState.h"
#include "rndobj/Shader.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/ShaderOptions.h"

NgFur::NgFur() {}

bool NgFur::Prep(RndMesh *, RndMat *) const {
    TheShaderMgr.SetPConstant((PShaderConstant)12, mFurDetail);
    TheRenderState.SetTextureFilter(12, RndRenderState::kFilterModeLinear, false);
    return true;
}

bool NgFur::Shell(int i1, RndMesh *mesh, RndMat *mat) const {
    float f3;
    float f4;

    if (i1 != 0) {
        f4 = (float)i1 / (float)(mLayers - 1);
    } else {
        f4 = 0;
    }
    if (i1 != 0) {
        f3 = powf(f4, mCurvature);
    } else {
        f3 = 0;
    }
    Vector4 v90;
    v90.x = mStretch * f4;
    v90.y = mSlide * f3;
    v90.z = mGravity * mStretch * f4;
    v90.w = mGravity * mSlide * f3;
    TheShaderMgr.SetVConstant((VShaderConstant)0x32, v90);

    Hmx::Color v80;
    Subtract(mEndsTint, mRootsTint, v80);
    Multiply(v80, f4, v80);
    Add(mRootsTint, v80, v80);
    TheShaderMgr.SetPConstant((PShaderConstant)0xC, v80.AsVector4());

    Vector4 v70;
    float exp = -(mShellOut * 0.7f - 1.0f);
    float x;
    if (i1 != 0) {
        x = mThickness * powf(f4, exp);
    } else {
        x = mThickness / (float)mLayers;
    }
    int bones = mesh->NumBones();
    if (bones > 1) {
        f4 = bones;
    } else {
        f4 = 1;
    }
    v70.x = x;
    v70.y = f4;
    v70.z = 0;
    v70.w = 0;
    TheShaderMgr.SetVConstant((VShaderConstant)0x33, v70);

    exp = mAlphaFalloff * 2 + 1;
    float f2;
    if (i1 != 0) {
        f2 = powf((float)i1 / (float)mLayers, exp);
    } else {
        f2 = 0;
    }
    Vector4 v60;
    float x2 = 1 / (1 - f2);
    v60.x = x2;
    v60.y = -(x2 * f2);
    v60.z = mFurTiling;
    v60.w = 0;
    TheShaderMgr.SetPConstant((PShaderConstant)0xB, v60);
    RndShader::SelectConfig(mat, kFurShader, false);
    if (i1 == 0) {
        TheRenderState.SetBlend(
            RndRenderState::kBlendOne,
            RndRenderState::kBlendZero,
            RndRenderState::kBlendOne,
            RndRenderState::kBlendOne
        );
        TheRenderState.SetDepthTestEnable(true);
        TheRenderState.SetDepthWriteEnable(true);
        TheRenderState.SetDepthFunc(RndRenderState::kTestFuncLess);
        NgMat::SetCurrent(nullptr);
    }
    return true;
}
