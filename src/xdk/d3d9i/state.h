#pragma once

#include <types.h>

extern "C" {
void D3DDevice_SetRenderState_AlphaBlendEnable(struct D3DDevice *, uint);
void D3DDevice_SetRenderState_BlendOp(struct D3DDevice *, uint);
uint D3DDevice_GetRenderState_BlendOp(struct D3DDevice *);
void D3DDevice_SetRenderState_SrcBlend(struct D3DDevice *, uint);
void D3DDevice_SetRenderState_DestBlend(struct D3DDevice *, uint);
void D3DDevice_SetRenderState_SrcBlendAlpha(struct D3DDevice *, uint);
void D3DDevice_SetRenderState_DestBlendAlpha(struct D3DDevice *, uint);
uint D3DDevice_GetRenderState_SrcBlend(struct D3DDevice *);
uint D3DDevice_GetRenderState_DestBlend(struct D3DDevice *);
uint D3DDevice_GetRenderState_SrcBlendAlpha(struct D3DDevice *);
uint D3DDevice_GetRenderState_DestBlendAlpha(struct D3DDevice *);
void D3DDevice_SetRenderState_ColorWriteEnable(struct D3DDevice *, uint);
void D3DDevice_SetRenderState_FillMode(struct D3DDevice *, uint);
void D3DDevice_SetRenderState_CullMode(struct D3DDevice *, uint);
void D3DDevice_SetRenderState_StencilEnable(struct D3DDevice *, uint);
void D3DDevice_SetRenderState_StencilFail(struct D3DDevice *, uint);
void D3DDevice_SetRenderState_StencilZFail(struct D3DDevice *, uint);
void D3DDevice_SetRenderState_StencilPass(struct D3DDevice *, uint);
}
