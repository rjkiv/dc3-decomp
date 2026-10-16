#pragma once

class RndRenderState {
public:
    enum ColorBits {
        kRed = 1,
        kGreen = 2,
        kBlue = 4,
        kAlpha = 8,
        kColorOnly = 7,
        kAllChannels = 15
    };
    // basically D3DBLEND
    enum Blend {
        kZero = 0x0000,
        kOne = 0x0001,
        kSrcColor = 0x0004,
        kInvSrcColor = 0x0005,
        kSrcAlpha = 0x0006,
        kInvSrcAlpha = 0x0007,
        kDestColor = 0x0008,
        kInvDestColor = 0x0009,
        kDestAlpha = 0x000a,
        kInvDestAlpha = 0x000b,
    };
    // basically D3DBLENDOP
    enum BlendOp {
        kAdd = 0x0000,
        kSub = 0x0001,
        kMin = 0x0002,
        kMax = 0x0003,
        kReverseSub = 0x0004,
    };
    // basically D3DFILLMODE
    enum FillMode {
        kWire = 0x0025,
        kSolid = 0x0000,
    };
    // basically D3DCULL
    enum CullMode {
        kNone = 0x0000,
        kCW = 0x0002,
        kCCW = 0x0006,
    };
    // basically D3DTEXTUREFILTERTYPE
    enum FilterMode {
        kPoint = 0x0000,
        kLinear = 0x0001,
    };
    // basically D3DTEXTUREADDRESS
    enum ClampMode {
        kWrap = 0x0000,
        kMirror = 0x0001,
        kClamp = 0x0002,
        kBorder = 0x0006,
    };
    // basically D3DSTENCILOP
    enum StencilOp {
        kKeep = 0x0000,
        kClear = 0x0001,
        kReplace = 0x0002,
        kIncSat = 0x0003,
        kDecSat = 0x0004,
        kInvert = 0x0005,
        kInc = 0x0006,
        kDec = 0x0007,
    };
    // basically D3DCMPFUNC
    enum TestFunc {
        kAlways = 0x0000,
        kLess = 0x0001,
        kLessEq = 0x0002,
        kEqual = 0x0003,
        kNotEqual = 0x0004,
        kGreater = 0x0005,
        kGreaterEq = 0x0006,
        kNever = 0x0007,
    };

    void SetBlendEnable(bool b);
    void SetBlendOp(BlendOp m);
    void SetBlend(Blend src, Blend dst, Blend srcAlpha, Blend dstAlpha);
    void SetColorWriteMask(unsigned int orBits);
    void SetTextureFilter(unsigned int idx, FilterMode m, bool);
    void SetTextureClamp(unsigned int idx, ClampMode u);
    void SetBorderColor(unsigned int idx, bool white);
    void SetFillMode(FillMode m);
    void SetCullMode(CullMode m);
    void SetAlphaTestEnable(bool b);
    void SetAlphaFunc(TestFunc m, unsigned int ref);
    void SetDepthTestEnable(bool b);
    void SetDepthWriteEnable(bool b);
    void SetDepthFunc(TestFunc m);
    void SetStencilTestEnable(bool b);
    void SetStencilFunc(TestFunc m, unsigned char ref);
    void SetStencilOp(StencilOp fail, StencilOp zfail, StencilOp pass);
    void Init(void);
};

extern RndRenderState TheRenderState;
