#pragma once

#include "xdk/d3d9i/d3dtypes.h"
#include "xdk/d3d9i/device.h"

extern "C" {

uint D3DDevice_GetRenderState_BlendOp(D3DDevice *);
uint D3DDevice_GetRenderState_SrcBlend(D3DDevice *);
uint D3DDevice_GetRenderState_DestBlend(D3DDevice *);
uint D3DDevice_GetRenderState_SrcBlendAlpha(D3DDevice *);
uint D3DDevice_GetRenderState_DestBlendAlpha(D3DDevice *);

void D3DDevice_SetRenderState_AlphaBlendEnable(D3DDevice *, uint);
void D3DDevice_SetRenderState_AlphaFunc(D3DDevice *, D3DCMPFUNC);
void D3DDevice_SetRenderState_AlphaRef(D3DDevice *, DWORD);
void D3DDevice_SetRenderState_AlphaTestEnable(D3DDevice *, uint);
void D3DDevice_SetRenderState_BlendOp(D3DDevice *, uint);
void D3DDevice_SetRenderState_SrcBlend(D3DDevice *, uint);
void D3DDevice_SetRenderState_DestBlend(D3DDevice *, uint);
void D3DDevice_SetRenderState_SrcBlendAlpha(D3DDevice *, uint);
void D3DDevice_SetRenderState_DestBlendAlpha(D3DDevice *, uint);
void D3DDevice_SetRenderState_ColorWriteEnable(D3DDevice *, uint);
void D3DDevice_SetRenderState_FillMode(D3DDevice *, uint);
void D3DDevice_SetRenderState_CullMode(D3DDevice *, uint);
void D3DDevice_SetRenderState_StencilEnable(D3DDevice *, uint);
void D3DDevice_SetRenderState_StencilFail(D3DDevice *, uint);
void D3DDevice_SetRenderState_StencilZFail(D3DDevice *, uint);
void D3DDevice_SetRenderState_StencilPass(D3DDevice *, uint);
void D3DDevice_SetRenderState_StencilFunc(D3DDevice *, D3DCMPFUNC);
void D3DDevice_SetRenderState_StencilRef(D3DDevice *, DWORD);
void D3DDevice_SetRenderState_ZEnable(D3DDevice *, DWORD);
void D3DDevice_SetRenderState_ZFunc(D3DDevice *, D3DCMPFUNC);
void D3DDevice_SetRenderState_ZWriteEnable(D3DDevice *, DWORD);
}
