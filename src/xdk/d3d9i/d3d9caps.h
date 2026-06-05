#pragma once
#include "d3d9types.h"
#include "xdk/xapilibi/xbase.h"

// Cap related definitions go here.

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _D3DVSHADERCAPS2_0 { /* Size=0x10 */
    /* 0x0000 */ UINT Caps;
    /* 0x0004 */ INT DynamicFlowControlDepth;
    /* 0x0008 */ INT NumTemps;
    /* 0x000c */ INT StaticFlowControlDepth;
} D3DVSHADERCAPS2_0;

typedef struct _D3DPSHADERCAPS2_0 { /* Size=0x14 */
    /* 0x0000 */ UINT Caps;
    /* 0x0004 */ INT DynamicFlowControlDepth;
    /* 0x0008 */ INT NumTemps;
    /* 0x000c */ INT StaticFlowControlDepth;
    /* 0x0010 */ INT NumInstructionSlots;
} D3DPSHADERCAPS2_0;

typedef struct _D3DCAPS9 { /* Size=0x130 */
    /* 0x0000 */ D3DDEVTYPE DeviceType;
    /* 0x0004 */ UINT AdapterOrdinal;
    /* 0x0008 */ UINT Caps;
    /* 0x000c */ UINT Caps2;
    /* 0x0010 */ UINT Caps3;
    /* 0x0014 */ UINT PresentationIntervals;
    /* 0x0018 */ UINT CursorCaps;
    /* 0x001c */ UINT DevCaps;
    /* 0x0020 */ UINT PrimitiveMiscCaps;
    /* 0x0024 */ UINT RasterCaps;
    /* 0x0028 */ UINT ZCmpCaps;
    /* 0x002c */ UINT SrcBlendCaps;
    /* 0x0030 */ UINT DestBlendCaps;
    /* 0x0034 */ UINT AlphaCmpCaps;
    /* 0x0038 */ UINT ShadeCaps;
    /* 0x003c */ UINT TextureCaps;
    /* 0x0040 */ UINT TextureFilterCaps;
    /* 0x0044 */ UINT CubeTextureFilterCaps;
    /* 0x0048 */ UINT VolumeTextureFilterCaps;
    /* 0x004c */ UINT TextureAddressCaps;
    /* 0x0050 */ UINT VolumeTextureAddressCaps;
    /* 0x0054 */ UINT LineCaps;
    /* 0x0058 */ UINT MaxTextureWidth;
    /* 0x005c */ UINT MaxTextureHeight;
    /* 0x0060 */ UINT MaxVolumeExtent;
    /* 0x0064 */ UINT MaxTextureRepeat;
    /* 0x0068 */ UINT MaxTextureAspectRatio;
    /* 0x006c */ UINT MaxAnisotropy;
    /* 0x0070 */ float MaxVertexW;
    /* 0x0074 */ float GuardBandLeft;
    /* 0x0078 */ float GuardBandTop;
    /* 0x007c */ float GuardBandRight;
    /* 0x0080 */ float GuardBandBottom;
    /* 0x0084 */ float ExtentsAdjust;
    /* 0x0088 */ UINT StencilCaps;
    /* 0x008c */ UINT FVFCaps;
    /* 0x0090 */ UINT TextureOpCaps;
    /* 0x0094 */ UINT MaxTextureBlendStages;
    /* 0x0098 */ UINT MaxSimultaneousTextures;
    /* 0x009c */ UINT VertexProcessingCaps;
    /* 0x00a0 */ UINT MaxActiveLights;
    /* 0x00a4 */ UINT MaxUserClipPlanes;
    /* 0x00a8 */ UINT MaxVertexBlendMatrices;
    /* 0x00ac */ UINT MaxVertexBlendMatrixIndex;
    /* 0x00b0 */ float MaxPointSize;
    /* 0x00b4 */ UINT MaxPrimitiveCount;
    /* 0x00b8 */ UINT MaxVertexIndex;
    /* 0x00bc */ UINT MaxStreams;
    /* 0x00c0 */ UINT MaxStreamStride;
    /* 0x00c4 */ UINT VertexShaderVersion;
    /* 0x00c8 */ UINT MaxVertexShaderConst;
    /* 0x00cc */ UINT PixelShaderVersion;
    /* 0x00d0 */ float PixelShader1xMaxValue;
    /* 0x00d4 */ UINT DevCaps2;
    /* 0x00d8 */ float MaxNpatchTessellationLevel;
    /* 0x00dc */ UINT Reserved5;
    /* 0x00e0 */ UINT MasterAdapterOrdinal;
    /* 0x00e4 */ UINT AdapterOrdinalInGroup;
    /* 0x00e8 */ UINT NumberOfAdaptersInGroup;
    /* 0x00ec */ UINT DeclTypes;
    /* 0x00f0 */ UINT NumSimultaneousRTs;
    /* 0x00f4 */ UINT StretchRectFilterCaps;
    /* 0x00f8 */ D3DVSHADERCAPS2_0 VS20Caps;
    /* 0x0108 */ D3DPSHADERCAPS2_0 PS20Caps;
    /* 0x011c */ UINT VertexTextureFilterCaps;
    /* 0x0120 */ UINT MaxVShaderInstructionsExecuted;
    /* 0x0124 */ UINT MaxPShaderInstructionsExecuted;
    /* 0x0128 */ UINT MaxVertexShader30InstructionSlots;
    /* 0x012c */ UINT MaxPixelShader30InstructionSlots;
} D3DCAPS9;

#ifdef __cplusplus
}
#endif
