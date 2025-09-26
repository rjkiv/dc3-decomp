#pragma once
#include <xdk/D3D9.h>
#include <types.h>
class RndRenderState {
public:
    enum Blend {
    };
    enum BlendOp {
    };
    enum FillMode {
    };
    enum CullMode {
    };
    enum FilterMode {
    };
    enum ClampMode {
    };
    enum StencilOp {
    };
    enum TestFunc {
    };
    static D3DCMPFUNC tf2cf[];

    void SetBlendEnable(bool);
    void SetBlendOp(BlendOp);
    void SetBlend(Blend, Blend, Blend, Blend);
    void SetColorWriteMask(uint);
    void SetTextureFilter(uint, FilterMode, bool);
    void SetTextureClamp(uint, ClampMode);
    void SetBorderColor(uint, bool);
    void SetFillMode(FillMode);
    void SetCullMode(CullMode);
    void SetAlphaTestEnable(bool);
    void SetAlphaFunc(TestFunc, uint);
    void SetDepthTestEnable(bool);
    void SetDepthWriteEnable(bool);
    void SetDepthFunc(TestFunc);
    void SetStencilTestEnable(bool);
    void SetStencilFunc(TestFunc, u8);
    void SetStencilOp(StencilOp fail, StencilOp zfail, StencilOp pass);
    void Init(void);
};

extern RndRenderState TheRenderState;
