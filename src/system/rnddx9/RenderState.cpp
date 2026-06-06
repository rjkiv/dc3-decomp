#include "RenderState.h"
#include "rnddx9/Rnd.h"
#include "xdk/d3d9i/d3d9.h"
#include "xdk/d3d9i/d3d9types.h"

RndRenderState TheRenderState;

// void RndRenderState::SetTextureFilter(uint, FilterMode, bool) {}

// void RndRenderState::SetTextureClamp(uint, ClampMode) {}

void RndRenderState::SetBorderColor(uint sampler, bool black_or_white) {
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

void RndRenderState::SetColorWriteMask(uint mask) {
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
    TheDxRnd.Device()->SetRenderState(D3DRS_ALPHAFUNC, tf2cf[tf]);
}

void RndRenderState::SetDepthTestEnable(bool b) {
    TheDxRnd.Device()->SetRenderState(D3DRS_ZENABLE, b);
}
void RndRenderState::SetDepthWriteEnable(bool b) {
    TheDxRnd.Device()->SetRenderState(D3DRS_ZWRITEENABLE, b);
}

void RndRenderState::SetDepthFunc(TestFunc tf) {
    TheDxRnd.Device()->SetRenderState(D3DRS_ZFUNC, tf2cf[TheDxRnd.Unk301() * 8 + tf]);
}

void RndRenderState::SetStencilTestEnable(bool b) {
    TheDxRnd.Device()->SetRenderState(D3DRS_STENCILENABLE, b);
}

void RndRenderState::SetStencilFunc(TestFunc tf, u8 ref) {
    TheDxRnd.Device()->SetRenderState(D3DRS_STENCILREF, ref);
    TheDxRnd.Device()->SetRenderState(
        D3DRS_STENCILFUNC, tf2cf[TheDxRnd.Unk301() * 8 + tf]
    );
}

void RndRenderState::SetStencilOp(StencilOp fail, StencilOp zfail, StencilOp pass) {
    TheDxRnd.Device()->SetRenderState(D3DRS_STENCILFAIL, fail);
    TheDxRnd.Device()->SetRenderState(D3DRS_STENCILZFAIL, zfail);
    TheDxRnd.Device()->SetRenderState(D3DRS_STENCILPASS, pass);
}

void RndRenderState::Init() {
    SetTextureClamp(4, (ClampMode)2);
    SetTextureClamp(5, (ClampMode)2);
    SetTextureFilter(5, (FilterMode)1, false);
}
