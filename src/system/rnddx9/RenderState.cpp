#include "rndobj/RenderState.h"
#include "rnddx9/Rnd.h"
#include "xdk/D3D9.h"

RndRenderState TheRenderState;

static const D3DCMPFUNC sFuncs[] = {
    D3DCMP_ALWAYS,   D3DCMP_LESS,    D3DCMP_LESSEQUAL,    D3DCMP_EQUAL,
    D3DCMP_NOTEQUAL, D3DCMP_GREATER, D3DCMP_GREATEREQUAL, D3DCMP_NEVER,
    D3DCMP_ALWAYS,   D3DCMP_GREATER, D3DCMP_GREATEREQUAL, D3DCMP_EQUAL,
    D3DCMP_NOTEQUAL, D3DCMP_LESS,    D3DCMP_LESSEQUAL,    D3DCMP_NEVER
};

static const D3DCMPFUNC sAlphaFuncs[] = {
    D3DCMP_ALWAYS,   D3DCMP_LESS,    D3DCMP_LESSEQUAL,    D3DCMP_EQUAL,
    D3DCMP_NOTEQUAL, D3DCMP_GREATER, D3DCMP_GREATEREQUAL, D3DCMP_NEVER,
};

void RndRenderState::SetTextureFilter(uint idx, FilterMode m, bool __formal) {
    TheDxRnd.Device()->SetSamplerState(idx, D3DSAMP_MINFILTER, m);
    TheDxRnd.Device()->SetSamplerState(idx, D3DSAMP_MAGFILTER, m);
    TheDxRnd.Device()->SetSamplerState(idx, D3DSAMP_MIPFILTER, m);
}

void RndRenderState::SetTextureClamp(uint idx, ClampMode u) {
    TheDxRnd.Device()->SetSamplerState(idx, D3DSAMP_ADDRESSU, u);
    TheDxRnd.Device()->SetSamplerState(idx, D3DSAMP_ADDRESSV, u);
    TheDxRnd.Device()->SetSamplerState(idx, D3DSAMP_ADDRESSW, u);
}

void RndRenderState::SetBorderColor(uint idx, bool white) {
    TheDxRnd.Device()->SetSamplerState(idx, D3DSAMP_BORDERCOLOR, white);
}

void RndRenderState::SetBlendEnable(bool b) {
    TheDxRnd.Device()->SetRenderState(D3DRS_ALPHABLENDENABLE, b);
}

void RndRenderState::SetBlendOp(BlendOp m) {
    TheDxRnd.Device()->SetRenderState(D3DRS_BLENDOP, m);
}

void RndRenderState::SetBlend(Blend src, Blend dst, Blend srcAlpha, Blend dstAlpha) {
    TheDxRnd.Device()->SetRenderState(D3DRS_SRCBLEND, src);
    TheDxRnd.Device()->SetRenderState(D3DRS_DESTBLEND, dst);
    TheDxRnd.Device()->SetRenderState(D3DRS_SRCBLENDALPHA, srcAlpha);
    TheDxRnd.Device()->SetRenderState(D3DRS_DESTBLENDALPHA, dstAlpha);
}

void RndRenderState::SetColorWriteMask(uint orBits) {
    TheDxRnd.Device()->SetRenderState(D3DRS_COLORWRITEENABLE, orBits);
}

void RndRenderState::SetFillMode(FillMode m) {
    TheDxRnd.Device()->SetRenderState(D3DRS_FILLMODE, m);
}

void RndRenderState::SetCullMode(CullMode m) {
    TheDxRnd.Device()->SetRenderState(D3DRS_CULLMODE, m);
}

void RndRenderState::SetAlphaTestEnable(bool b) {
    TheDxRnd.Device()->SetRenderState(D3DRS_ALPHATESTENABLE, b);
}

void RndRenderState::SetAlphaFunc(TestFunc m, uint ref) {
    TheDxRnd.Device()->SetRenderState(D3DRS_ALPHAREF, ref);
    TheDxRnd.Device()->SetRenderState(D3DRS_ALPHAFUNC, sAlphaFuncs[m]);
}

void RndRenderState::SetDepthTestEnable(bool b) {
    TheDxRnd.Device()->SetRenderState(D3DRS_ZENABLE, b);
}
void RndRenderState::SetDepthWriteEnable(bool b) {
    TheDxRnd.Device()->SetRenderState(D3DRS_ZWRITEENABLE, b);
}

void RndRenderState::SetDepthFunc(TestFunc m) {
    TheDxRnd.Device()->SetRenderState(D3DRS_ZFUNC, sFuncs[TheDxRnd.InverseZ() * 8 + m]);
}

void RndRenderState::SetStencilTestEnable(bool b) {
    TheDxRnd.Device()->SetRenderState(D3DRS_STENCILENABLE, b);
}

void RndRenderState::SetStencilFunc(TestFunc m, u8 ref) {
    TheDxRnd.Device()->SetRenderState(D3DRS_STENCILREF, ref);
    TheDxRnd.Device()->SetRenderState(
        D3DRS_STENCILFUNC, sFuncs[TheDxRnd.InverseZ() * 8 + m]
    );
}

void RndRenderState::SetStencilOp(StencilOp fail, StencilOp zFail, StencilOp pass) {
    TheDxRnd.Device()->SetRenderState(D3DRS_STENCILFAIL, fail);
    TheDxRnd.Device()->SetRenderState(D3DRS_STENCILZFAIL, zFail);
    TheDxRnd.Device()->SetRenderState(D3DRS_STENCILPASS, pass);
}

void RndRenderState::Init() {
    SetTextureClamp(4, kClamp);
    SetTextureClamp(5, kClamp);
    SetTextureFilter(5, kLinear, false);
}
