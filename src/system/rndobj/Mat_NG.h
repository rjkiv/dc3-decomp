#pragma once
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

    float unk22c;
    float unk230;
    float unk234;
    float unk238;
    RndRenderState::Blend unk23c; // 0x23c
    RndRenderState::Blend unk240; // 0x240
    bool mDepthTestEnable; // 0x244
    bool mDepthWriteEnable; // 0x245
    RndRenderState::TestFunc mDepthFunc; // 0x248
    RndRenderState::TestFunc mStencilFunc; // 0x24c
    RndRenderState::StencilOp unk250; // 0x250
    int unk254;
    int unk258;
    int unk25c;
    int unk260;
    int unk264;
    int unk268;
    int unk26c;
    int unk270;
    int unk274;
    int unk278;
    int unk27c;
    int unk280;
    int unk284;
    int unk288;
    int unk28c;
    int unk290;
    int unk294;
    int unk298;
    int unk29c;
    int unk2a0;
    int unk2a4;
    int unk2a8;
    int unk2ac;
    int unk2b0;
    int unk2b4;
    int unk2b8;
    int unk2bc;
    int unk2c0;
    int unk2c4;
    int unk2c8;
    int unk2cc;
    int unk2d0;
    int unk2d4;
    float unk2d8;
    float unk2dc;
    float unk2e0;
    float unk2e4;
    RndRenderState::BlendOp mBlendOp; // 0x2e8
    bool mBlendEnable; // 0x2ec
};
