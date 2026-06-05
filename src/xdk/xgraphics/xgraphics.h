#pragma once
#include "xdk/XAPILIB.h"
#include "xdk/D3D9.h"
#include "xdk/win_types.h"
// This is where Xbox related functions that use D3D9 stuff will go.

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XGTEXTURE_DESC { /* Size=0x3c */
    /* 0x0000 */ D3DRESOURCETYPE ResourceType;
    /* 0x0004 */ UINT Width;
    /* 0x0008 */ UINT Height;
    /* 0x000c */ UINT Depth;
    /* 0x0010 */ D3DFORMAT Format;
    /* 0x0014 */ UINT RowPitch;
    /* 0x0018 */ UINT SlicePitch;
    /* 0x001c */ UINT BitsPerPixel;
    /* 0x0020 */ UINT WidthInBlocks;
    /* 0x0024 */ UINT HeightInBlocks;
    /* 0x0028 */ UINT DepthInBlocks;
    /* 0x002c */ UINT BytesPerBlock;
    /* 0x0030 */ INT ExpBias;
    /* 0x0034 */ UINT Flags;
    /* 0x0038 */ D3DMULTISAMPLE_TYPE MultiSampleType;
} XGTEXTURE_DESC;

typedef struct _XGLAYOUT_REGION { /* Size=0x8 */
    /* 0x0000 */ UINT StartOffset;
    /* 0x0004 */ UINT EndOffset;
} XGLAYOUT_REGION;

typedef struct _XGIDEALSHADERCOST { /* Size=0x54 */
    /* 0x0000 */ float MinAlu;
    /* 0x0004 */ float MaxAlu;
    /* 0x0008 */ float Interpolator;
    /* 0x000c */ float MinTexture;
    /* 0x0010 */ float MaxTexture;
    /* 0x0014 */ float MinVertex;
    /* 0x0018 */ float MaxVertex;
    /* 0x001c */ float Sequencer;
    /* 0x0020 */ float MinOverall;
    /* 0x0024 */ float MaxOverall;
    /* 0x0028 */ BOOL bHasHitUnknownControlFlow;
    /* 0x002c */ BOOL bHasHitChangeableControlFlow;
    /* 0x0030 */ BOOL bHasHitPredicatedJump;
    /* 0x0034 */ BOOL bHasHitPredicatedEndloop;
    /* 0x0038 */ BOOL bHasHitUnknownFetchConstant;
    /* 0x003c */ BOOL bHasHitUnpatchedVfetch;
    /* 0x0040 */ INT MaxTempReg;
    /* 0x0044 */ float AvgTcInstructions;
    /* 0x0048 */ float AvgTcAndTcCfInstructions;
    /* 0x004c */ float AvgVcInstructions;
    /* 0x0050 */ float AvgVcAndVcCfInstructions;
} XGIDEALSHADERCOST;

UINT XGSurfaceSize(
    UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample
);
UINT XGHierarchicalZSize(UINT Width, UINT Height, D3DMULTISAMPLE_TYPE MultiSample);
VOID XGGetTextureDesc(D3DBaseTexture *pTexture, UINT Level, XGTEXTURE_DESC *pDesc);
VOID XGTileTextureLevel(
    UINT Width,
    UINT Height,
    UINT Level,
    DWORD GpuFormat,
    DWORD Flags,
    VOID *pDestination,
    const tagPOINT *pPoint,
    const VOID *pSource,
    UINT RowPitch,
    const RECT *pRect
);

void XGGetTextureLayout(
    D3DBaseTexture *pTexture,
    UINT *pBaseData,
    UINT *pBaseSize,
    XGLAYOUT_REGION *pBaseRegionList,
    UINT *pBaseRegionListCount,
    UINT BaseRegionAlignment,
    UINT *pMipData,
    UINT *pMipSize,
    XGLAYOUT_REGION *pMipRegionList,
    UINT *pMipRegionListCount,
    UINT MipRegionAlignment
);

INT XGEstimateIdealShaderCost(
    const void *pFunction, UINT pass, XGIDEALSHADERCOST *pShaderCost
);

void XGRegisterPixelShader(D3DPixelShader *pShader, void *pPhysicalPart);
void XGRegisterVertexShader(D3DVertexShader *pShader, void *pPhysicalPart);

#ifdef __cplusplus
}
#endif
