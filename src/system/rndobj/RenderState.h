#pragma once

class RndRenderState {
public:
    // basically D3DBLEND
    enum Blend {
        kBlendZero = 0x0000,
        kBlendOne = 0x0001,
        kBlendSrcColor = 0x0004,
        kBlendInvSrcColor = 0x0005,
        kBlendSrcAlpha = 0x0006,
        kBlendInvSrcAlpha = 0x0007,
        kBlendDestColor = 0x0008,
        kBlendInvDestColor = 0x0009,
        kBlendDestAlpha = 0x000a,
        kBlendInvDestAlpha = 0x000b,
        kBlendBlendFactor = 0x000c,
        kBlendInvBlendFactor = 0x000d,
        kBlendConstantAlpha = 0x000e,
        kBlendInvConstantAlpha = 0x000f,
        kBlendSrcAlphaSat = 0x0010,
    };
    // basically D3DBLENDOP
    enum BlendOp {
        kBlendOpAdd = 0x0000,
        kBlendOpSubtract = 0x0001,
        kBlendOpMin = 0x0002,
        kBlendOpMax = 0x0003,
        kBlendOpRevSubtract = 0x0004,
    };
    // basically D3DFILLMODE
    enum FillMode {
        kFillModePoint = 0x0001,
        kFillModeWireframe = 0x0025,
        kFillModeSolid = 0x0000,
    };
    // basically D3DCULL
    enum CullMode {
        kCullModeNone = 0x0000,
        kCullModeCW = 0x0002,
        kCullModeCCW = 0x0006,
    };
    // basically D3DTEXTUREFILTERTYPE
    enum FilterMode {
        kFilterModePoint = 0x0000,
        kFilterModeLinear = 0x0001,
        kFilterModeNone = 0x0002,
        kFilterModeAnisotropic = 0x0004,
    };
    // basically D3DTEXTUREADDRESS
    enum ClampMode {
        kClampModeWrap = 0x0000,
        kClampModeMirror = 0x0001,
        kClampModeClamp = 0x0002,
        kClampModeMirrorOnce = 0x0003,
        kClampModeBorderHalf = 0x0004,
        kClampModeMirrorOnceBorderHalf = 0x0005,
        kClampModeBorder = 0x0006,
        kClampModeMirrorOnceBorder = 0x0007,
    };
    // basically D3DSTENCILOP
    enum StencilOp {
        kStencilOpKeep = 0x0000,
        kStencilOpZero = 0x0001,
        kStencilOpReplace = 0x0002,
        kStencilOpIncrSat = 0x0003,
        kStencilOpDecrSat = 0x0004,
        kStencilOpInvert = 0x0005,
        kStencilOpIncr = 0x0006,
        kStencilOpDecr = 0x0007,
    };
    // basically D3DCMPFUNC
    enum TestFunc {
        kTestFuncAlways = 0x0000,
        kTestFuncLess = 0x0001,
        kTestFuncLessEqual = 0x0002,
        kTestFuncEqual = 0x0003,
        kTestFuncNotEqual = 0x0004,
        kTestFuncGreater = 0x0005,
        kTestFuncGreaterEqual = 0x0006,
        kTestFuncNever = 0x0007,
    };

    void SetBlendEnable(bool);
    void SetBlendOp(BlendOp);
    void SetBlend(Blend, Blend, Blend, Blend);
    void SetColorWriteMask(unsigned int);
    void SetTextureFilter(unsigned int, FilterMode, bool);
    void SetTextureClamp(unsigned int, ClampMode);
    void SetBorderColor(unsigned int, bool);
    void SetFillMode(FillMode);
    void SetCullMode(CullMode);
    void SetAlphaTestEnable(bool);
    void SetAlphaFunc(TestFunc, unsigned int);
    void SetDepthTestEnable(bool);
    void SetDepthWriteEnable(bool);
    void SetDepthFunc(TestFunc);
    void SetStencilTestEnable(bool);
    void SetStencilFunc(TestFunc, unsigned char);
    void SetStencilOp(StencilOp fail, StencilOp zfail, StencilOp pass);
    void Init(void);
};

extern RndRenderState TheRenderState;
