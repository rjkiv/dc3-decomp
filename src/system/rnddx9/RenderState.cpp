#include "RenderState.h"

#include "rnddx9/Rnd_Xbox.h"

RndRenderState TheRenderState;

void RndRenderState::SetBlendEnable(bool b) {
    D3DDevice_SetRenderState_AlphaBlendEnable(TheDxRnd.D3DDevice(), (u8)b);
}

void RndRenderState::SetBlendOp(BlendOp op) {
    D3DDevice_SetRenderState_BlendOp(TheDxRnd.D3DDevice(), (int)op);
}

void RndRenderState::SetBlend(
    Blend srcblend, Blend dstblend, Blend srcblenda, Blend dstblenda
) {
    D3DDevice_SetRenderState_SrcBlend(TheDxRnd.D3DDevice(), (int)srcblend);
    D3DDevice_SetRenderState_DestBlend(TheDxRnd.D3DDevice(), (int)dstblend);
    D3DDevice_SetRenderState_SrcBlendAlpha(TheDxRnd.D3DDevice(), (int)srcblenda);
    D3DDevice_SetRenderState_DestBlendAlpha(TheDxRnd.D3DDevice(), (int)dstblenda);
}

void RndRenderState::SetColorWriteMask(uint mask) {
    D3DDevice_SetRenderState_ColorWriteEnable(TheDxRnd.D3DDevice(), mask);
}

// void RndRenderState::SetTextureFilter(unsigned int, FilterMode, bool) {}
// void RndRenderState::SetTextureClamp(unsigned int, ClampMode) {}
// void RndRenderState::SetBorderColor(unsigned int, bool) {}

void RndRenderState::SetFillMode(FillMode mode) {
    D3DDevice_SetRenderState_FillMode(TheDxRnd.D3DDevice(), (int)mode);
}

void RndRenderState::SetCullMode(CullMode mode) {
    D3DDevice_SetRenderState_CullMode(TheDxRnd.D3DDevice(), (int)mode);
}

void RndRenderState::SetAlphaTestEnable(bool b) {
    D3DDevice_SetRenderState_AlphaTestEnable(TheDxRnd.D3DDevice(), b);
}

void RndRenderState::SetAlphaFunc(TestFunc tf, unsigned int ref) {
    D3DDevice_SetRenderState_AlphaRef(TheDxRnd.D3DDevice(), ref);
    D3DDevice_SetRenderState_AlphaFunc(TheDxRnd.D3DDevice(), tf2cf[tf]);
}

void RndRenderState::SetDepthTestEnable(bool b) {
    D3DDevice_SetRenderState_ZEnable(TheDxRnd.D3DDevice(), b);
}
void RndRenderState::SetDepthWriteEnable(bool b) {
    D3DDevice_SetRenderState_ZWriteEnable(TheDxRnd.D3DDevice(), b);
}

void RndRenderState::SetDepthFunc(TestFunc tf) {
    D3DDevice_SetRenderState_ZFunc(
        TheDxRnd.D3DDevice(), tf2cf[TheDxRnd.unk_0x301 * 8 + tf]
    );
}

void RndRenderState::SetStencilTestEnable(bool b) {
    D3DDevice_SetRenderState_StencilEnable(TheDxRnd.D3DDevice(), (u8)b);
}

void RndRenderState::SetStencilFunc(TestFunc tf, u8 ref) {
    D3DDevice_SetRenderState_StencilRef(TheDxRnd.D3DDevice(), ref);
    D3DDevice_SetRenderState_StencilFunc(
        TheDxRnd.D3DDevice(), tf2cf[TheDxRnd.unk_0x301 * 8 + tf]
    );
}

void RndRenderState::SetStencilOp(StencilOp fail, StencilOp zfail, StencilOp pass) {
    D3DDevice_SetRenderState_StencilFail(TheDxRnd.D3DDevice(), (int)fail);
    D3DDevice_SetRenderState_StencilZFail(TheDxRnd.D3DDevice(), (int)zfail);
    D3DDevice_SetRenderState_StencilPass(TheDxRnd.D3DDevice(), (int)pass);
}

void RndRenderState::Init(void) {
    SetTextureClamp(4, (ClampMode)2);
    SetTextureClamp(5, (ClampMode)2);

    SetTextureFilter(5, (FilterMode)1, false);
}
