#include "RenderState.h"
#include "rnddx9/Rnd.h"
#include "xdk/d3d9i/d3d9.h"

RndRenderState TheRenderState;

void RndRenderState::SetTextureFilter(uint, FilterMode, bool) {}

void RndRenderState::SetTextureClamp(uint, ClampMode) {}

void RndRenderState::SetBorderColor(uint sampler, bool black_or_white) {
    D3DDevice_SetSamplerState_BorderColor(
        TheDxRnd.Device(), (DWORD)sampler, black_or_white
    );
}

void RndRenderState::SetBlendEnable(bool b) {
    D3DDevice_SetRenderState_AlphaBlendEnable(TheDxRnd.Device(), (u8)b);
}

void RndRenderState::SetBlendOp(BlendOp op) {
    D3DDevice_SetRenderState_BlendOp(TheDxRnd.Device(), (int)op);
}

void RndRenderState::SetBlend(
    Blend srcblend, Blend dstblend, Blend srcblenda, Blend dstblenda
) {
    D3DDevice_SetRenderState_SrcBlend(TheDxRnd.Device(), (int)srcblend);
    D3DDevice_SetRenderState_DestBlend(TheDxRnd.Device(), (int)dstblend);
    D3DDevice_SetRenderState_SrcBlendAlpha(TheDxRnd.Device(), (int)srcblenda);
    D3DDevice_SetRenderState_DestBlendAlpha(TheDxRnd.Device(), (int)dstblenda);
}

void RndRenderState::SetColorWriteMask(uint mask) {
    D3DDevice_SetRenderState_ColorWriteEnable(TheDxRnd.Device(), mask);
}

void RndRenderState::SetFillMode(FillMode mode) {
    D3DDevice_SetRenderState_FillMode(TheDxRnd.Device(), (int)mode);
}

void RndRenderState::SetCullMode(CullMode mode) {
    D3DDevice_SetRenderState_CullMode(TheDxRnd.Device(), (int)mode);
}

void RndRenderState::SetAlphaTestEnable(bool b) {
    D3DDevice_SetRenderState_AlphaTestEnable(TheDxRnd.Device(), b);
}

void RndRenderState::SetAlphaFunc(TestFunc tf, unsigned int ref) {
    D3DDevice_SetRenderState_AlphaRef(TheDxRnd.Device(), ref);
    D3DDevice_SetRenderState_AlphaFunc(TheDxRnd.Device(), tf2cf[tf]);
}

void RndRenderState::SetDepthTestEnable(bool b) {
    D3DDevice_SetRenderState_ZEnable(TheDxRnd.Device(), b);
}
void RndRenderState::SetDepthWriteEnable(bool b) {
    D3DDevice_SetRenderState_ZWriteEnable(TheDxRnd.Device(), b);
}

void RndRenderState::SetDepthFunc(TestFunc tf) {
    D3DDevice_SetRenderState_ZFunc(TheDxRnd.Device(), tf2cf[TheDxRnd.Unk301() * 8 + tf]);
}

void RndRenderState::SetStencilTestEnable(bool b) {
    D3DDevice_SetRenderState_StencilEnable(TheDxRnd.Device(), (u8)b);
}

void RndRenderState::SetStencilFunc(TestFunc tf, u8 ref) {
    D3DDevice_SetRenderState_StencilRef(TheDxRnd.Device(), ref);
    D3DDevice_SetRenderState_StencilFunc(
        TheDxRnd.Device(), tf2cf[TheDxRnd.Unk301() * 8 + tf]
    );
}

void RndRenderState::SetStencilOp(StencilOp fail, StencilOp zfail, StencilOp pass) {
    D3DDevice_SetRenderState_StencilFail(TheDxRnd.Device(), (int)fail);
    D3DDevice_SetRenderState_StencilZFail(TheDxRnd.Device(), (int)zfail);
    D3DDevice_SetRenderState_StencilPass(TheDxRnd.Device(), (int)pass);
}

void RndRenderState::Init(void) {
    SetTextureClamp(4, (ClampMode)2);
    SetTextureClamp(5, (ClampMode)2);

    SetTextureFilter(5, (FilterMode)1, false);
}
