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

void RndRenderState::SetTextureFilter(unsigned int sampler, FilterMode filter, bool b3) {
    TheDxRnd.Device()->SetSamplerState(sampler, D3DSAMP_MINFILTER, filter);
    TheDxRnd.Device()->SetSamplerState(sampler, D3DSAMP_MAGFILTER, filter);
    TheDxRnd.Device()->SetSamplerState(sampler, D3DSAMP_MIPFILTER, filter);
}

void RndRenderState::SetTextureClamp(unsigned int sampler, ClampMode clamp) {
    TheDxRnd.Device()->SetSamplerState(sampler, D3DSAMP_ADDRESSU, clamp);
    TheDxRnd.Device()->SetSamplerState(sampler, D3DSAMP_ADDRESSV, clamp);
    TheDxRnd.Device()->SetSamplerState(sampler, D3DSAMP_ADDRESSW, clamp);
}

void RndRenderState::SetBorderColor(unsigned int sampler, bool black_or_white) {
    TheDxRnd.Device()->SetSamplerState(sampler, D3DSAMP_BORDERCOLOR, black_or_white);
}

void RndRenderState::SetBlendEnable(bool b) {
    TheDxRnd.Device()->SetRenderState(D3DRS_ALPHABLENDENABLE, b);
}

void RndRenderState::SetBlendOp(BlendOp op) {
    TheDxRnd.Device()->SetRenderState(D3DRS_BLENDOP, op);
}

void RndRenderState::SetBlend(
    Blend srcblend, Blend dstblend, Blend srcblenda, Blend dstblenda
) {
    TheDxRnd.Device()->SetRenderState(D3DRS_SRCBLEND, srcblend);
    TheDxRnd.Device()->SetRenderState(D3DRS_DESTBLEND, dstblend);
    TheDxRnd.Device()->SetRenderState(D3DRS_SRCBLENDALPHA, srcblenda);
    TheDxRnd.Device()->SetRenderState(D3DRS_DESTBLENDALPHA, dstblenda);
}

void RndRenderState::SetColorWriteMask(unsigned int mask) {
    TheDxRnd.Device()->SetRenderState(D3DRS_COLORWRITEENABLE, mask);
}

void RndRenderState::SetFillMode(FillMode mode) {
    TheDxRnd.Device()->SetRenderState(D3DRS_FILLMODE, mode);
}

void RndRenderState::SetCullMode(CullMode mode) {
    TheDxRnd.Device()->SetRenderState(D3DRS_CULLMODE, mode);
}

void RndRenderState::SetAlphaTestEnable(bool b) {
    TheDxRnd.Device()->SetRenderState(D3DRS_ALPHATESTENABLE, b);
}

void RndRenderState::SetAlphaFunc(TestFunc tf, unsigned int ref) {
    TheDxRnd.Device()->SetRenderState(D3DRS_ALPHAREF, ref);
    TheDxRnd.Device()->SetRenderState(D3DRS_ALPHAFUNC, sAlphaFuncs[tf]);
}

void RndRenderState::SetDepthTestEnable(bool b) {
    TheDxRnd.Device()->SetRenderState(D3DRS_ZENABLE, b);
}
void RndRenderState::SetDepthWriteEnable(bool b) {
    TheDxRnd.Device()->SetRenderState(D3DRS_ZWRITEENABLE, b);
}

void RndRenderState::SetDepthFunc(TestFunc tf) {
    TheDxRnd.Device()->SetRenderState(D3DRS_ZFUNC, sFuncs[TheDxRnd.Unk301() * 8 + tf]);
}

void RndRenderState::SetStencilTestEnable(bool b) {
    TheDxRnd.Device()->SetRenderState(D3DRS_STENCILENABLE, b);
}

void RndRenderState::SetStencilFunc(TestFunc tf, unsigned char ref) {
    TheDxRnd.Device()->SetRenderState(D3DRS_STENCILREF, ref);
    TheDxRnd.Device()->SetRenderState(
        D3DRS_STENCILFUNC, sFuncs[TheDxRnd.Unk301() * 8 + tf]
    );
}

void RndRenderState::SetStencilOp(StencilOp fail, StencilOp zfail, StencilOp pass) {
    TheDxRnd.Device()->SetRenderState(D3DRS_STENCILFAIL, fail);
    TheDxRnd.Device()->SetRenderState(D3DRS_STENCILZFAIL, zfail);
    TheDxRnd.Device()->SetRenderState(D3DRS_STENCILPASS, pass);
}

void RndRenderState::Init() {
    SetTextureClamp(4, kClampModeClamp);
    SetTextureClamp(5, kClampModeClamp);
    SetTextureFilter(5, kFilterModeLinear, false);
}
