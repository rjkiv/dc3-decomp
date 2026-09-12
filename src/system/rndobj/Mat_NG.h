#pragma once
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rndobj/Mat.h"
#include "rndobj/RenderState.h"

class NgMat : public RndMat {
public:
    NgMat();
    virtual ~NgMat();
    OBJ_CLASSNAME(Mat);
    OBJ_SET_TYPE(Mat);

    bool AllowFog() const;
    bool AllowHDR() const;
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

    Vector4 unk22c;
    RndRenderState::Blend unk23c; // 0x23c
    RndRenderState::Blend unk240; // 0x240
    bool mDepthTestEnable; // 0x244
    bool mDepthWriteEnable; // 0x245
    RndRenderState::TestFunc mDepthFunc; // 0x248
    RndRenderState::TestFunc mStencilFunc; // 0x24c
    RndRenderState::StencilOp unk250; // 0x250
    Hmx::Matrix4 unk254;
    Hmx::Matrix4 unk294;
    int unk2d4; // 0x2d4 - some sort of state enum
    float unk2d8;
    float unk2dc;
    float unk2e0;
    float unk2e4;
    RndRenderState::BlendOp mBlendOp; // 0x2e8
    bool mBlendEnable; // 0x2ec
};
