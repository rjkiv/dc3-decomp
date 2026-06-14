#pragma once
#include "d3d9types.h"
#include "d3d9caps.h"
#include "ppcintrinsics.h"
#include "xdk/xapilibi/winerror.h"
#include "limits.h"

// Larger struct definitions and functions go here.

#ifdef __cplusplus
extern "C" {
#endif

// source for most if not all of this: the kinect joyride pdb
// using black ops 2 pdb as well since its xdk version is closer to ours

typedef void (*D3DCALLBACK)(UINT);
typedef void (*D3DVBLANKCALLBACK)(D3DVBLANKDATA *);
typedef void (*D3DSWAPCALLBACK)(D3DSWAPDATA *);

typedef struct _D3DConstants { /* Size=0x23a0 */
    union {
        /* 0x0000 */ GPUFETCH_CONSTANT Fetch[32];
        struct {
            /* 0x0000 */ GPUTEXTURE_FETCH_CONSTANT TextureFetch[26];
            /* 0x0270 */ GPUVERTEX_FETCH_CONSTANT VertexFetch[18];
        };
    };
    union {
        /* 0x0300 */ XMVECTOR Alu[512];
        struct {
            /* 0x0300 */ XMVECTOR VertexShaderF[256];
            /* 0x1300 */ XMVECTOR PixelShaderF[256];
        };
    };
    union {
        /* 0x2300 */ UINT Flow[40];
        struct {
            /* 0x2300 */ UINT VertexShaderB[4];
            /* 0x2310 */ UINT PixelShaderB[4];
            /* 0x2320 */ UINT VertexShaderI[16];
            /* 0x2360 */ UINT PixelShaderI[16];
        };
    };
} D3DConstants;

// Forward declarations
typedef struct Direct3D Direct3D;
typedef struct D3DDevice D3DDevice;
typedef struct D3DVertexDeclaration D3DVertexDeclaration;
typedef struct D3DVertexShader D3DVertexShader;
typedef struct D3DPixelShader D3DPixelShader;
typedef struct D3DResource D3DResource;
typedef struct D3DBaseTexture D3DBaseTexture;
typedef struct D3DTexture D3DTexture;
typedef struct D3DVolumeTexture D3DVolumeTexture;
typedef struct D3DCubeTexture D3DCubeTexture;
typedef struct D3DArrayTexture D3DArrayTexture;
typedef struct D3DLineTexture D3DLineTexture;
typedef struct D3DVertexBuffer D3DVertexBuffer;
typedef struct D3DIndexBuffer D3DIndexBuffer;
typedef struct D3DSurface D3DSurface;
typedef struct D3DVolume D3DVolume;
typedef struct D3DQuery D3DQuery;
typedef struct D3DPerfCounters D3DPerfCounters;
typedef struct D3DStateBlock D3DStateBlock;

#pragma region D3DPerfCounters

// C
ULONG D3DPerfCounters_AddRef(D3DPerfCounters *pThis);
ULONG D3DPerfCounters_Release(D3DPerfCounters *pThis);
VOID D3DPerfCounters_GetDevice(D3DPerfCounters *pThis, D3DDevice **ppDevice);
VOID D3DPerfCounters_BlockUntilNotBusy(D3DPerfCounters *pThis);
UINT D3DPerfCounters_GetNumPasses(D3DPerfCounters *pThis);
BOOL D3DPerfCounters_IsBusy(D3DPerfCounters *pThis);
HRESULT
D3DPerfCounters_GetValues(
    D3DPerfCounters *pThis, D3DPERFCOUNTER_VALUES *pValues, UINT PassIndex, UINT *pPassType
);

// C++
struct D3DPerfCounters { /* Size=0x1 */
    ULONG AddRef() { return D3DPerfCounters_AddRef(this); }
    ULONG Release() { return D3DPerfCounters_Release(this); }
    HRESULT GetDevice(D3DDevice **ppDevice) {
        D3DPerfCounters_GetDevice(this, ppDevice);
        return S_OK;
    }
    BOOL IsBusy() { return D3DPerfCounters_IsBusy(this); }
    VOID BlockUntilNotBusy() { D3DPerfCounters_BlockUntilNotBusy(this); }
    UINT GetNumPasses() { return D3DPerfCounters_GetNumPasses(this); }
    HRESULT GetValues(D3DPERFCOUNTER_VALUES *pValues, UINT PassIndex, UINT *pPassType) {
        return D3DPerfCounters_GetValues(this, pValues, PassIndex, pPassType);
    }
};

#pragma endregion
#pragma region Direct3D

// https://learn.microsoft.com/en-us/windows/win32/api/d3d9helper/nn-d3d9helper-idirect3d9

HRESULT Direct3D_GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9 *pCaps);
HRESULT
Direct3D_CreateDevice(
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    void *pUnused,
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters,
    D3DDevice **ppReturnedDeviceInterface
);

struct Direct3D { /* Size=0x1 */
    static ULONG AddRef();
    static ULONG Release();
    static UINT GetAdapterCount();
    static HRESULT
    GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9 *pIdentifier);
    static HRESULT CheckDeviceType(
        UINT iAdapter,
        D3DDEVTYPE DevType,
        D3DFORMAT DisplayFormat,
        D3DFORMAT BackBufferFormat,
        BOOL bWindowed
    );
    static HRESULT CheckDeviceFormat(
        UINT Adapter,
        D3DDEVTYPE DeviceType,
        D3DFORMAT AdapterFormat,
        DWORD Usage,
        D3DRESOURCETYPE RType,
        D3DFORMAT CheckFormat
    );
    static HRESULT CheckDeviceMultiSampleType(
        UINT Adapter,
        D3DDEVTYPE DeviceType,
        D3DFORMAT SurfaceFormat,
        BOOL Windowed,
        D3DMULTISAMPLE_TYPE MultiSampleType,
        DWORD *pQualityLevels
    );
    static HRESULT CheckDepthStencilMatch(
        UINT Adapter,
        D3DDEVTYPE DeviceType,
        D3DFORMAT AdapterFormat,
        D3DFORMAT RenderTargetFormat,
        D3DFORMAT DepthStencilFormat
    );
    static HRESULT CheckDeviceFormatConversion(
        UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat
    );
    static HRESULT GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9 *pCaps) {
        return Direct3D_GetDeviceCaps(Adapter, DeviceType, pCaps);
    }
    static HRESULT CreateDevice(
        UINT Adapter,
        D3DDEVTYPE DeviceType,
        void *hFocusWindow,
        DWORD BehaviorFlags,
        D3DPRESENT_PARAMETERS *pPresentationParameters,
        D3DDevice **ppReturnedDeviceInterface
    ) {
        return Direct3D_CreateDevice(
            Adapter,
            DeviceType,
            hFocusWindow,
            BehaviorFlags,
            pPresentationParameters,
            ppReturnedDeviceInterface
        );
    }
    static void QueryGpuVersion(D3DGPUVERSION *pVersion);
};

#pragma endregion
#pragma region D3DDevice

// https://learn.microsoft.com/en-us/windows/win32/api/d3d9helper/nn-d3d9helper-idirect3ddevice9

// C
void D3DDevice_EnablePerfCounters(D3DDevice *pDevice, BOOL Enable);
void D3DDevice_SetPerfCounterEvents(
    D3DDevice *pDevice, const D3DPERFCOUNTER_EVENTS *pEvents, DWORD Flags
);
void D3DDevice_QueryPerfCounters(
    D3DDevice *pDevice, D3DPerfCounters *pCounters, DWORD Flags
);

D3DPixelShader *D3DDevice_CreatePixelShader(const DWORD *pFunction);
D3DVertexShader *D3DDevice_CreateVertexShader(const DWORD *pFunction);

DWORD D3DDevice_GetRenderState_BlendOp(D3DDevice *pDevice);
DWORD D3DDevice_GetRenderState_SrcBlend(D3DDevice *pDevice);
DWORD D3DDevice_GetRenderState_DestBlend(D3DDevice *pDevice);
DWORD D3DDevice_GetRenderState_SrcBlendAlpha(D3DDevice *pDevice);
DWORD D3DDevice_GetRenderState_DestBlendAlpha(D3DDevice *pDevice);

void D3DDevice_SetRenderState_AlphaBlendEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_AlphaFunc(D3DDevice *pDevice, D3DCMPFUNC Value);
void D3DDevice_SetRenderState_AlphaRef(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_AlphaTestEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_BlendOp(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_BlendFactor(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_SrcBlend(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_DestBlend(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_SrcBlendAlpha(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_DestBlendAlpha(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_ColorWriteEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_FillMode(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_CullMode(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_StencilEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_StencilFail(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_StencilZFail(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_StencilPass(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_StencilFunc(D3DDevice *pDevice, D3DCMPFUNC Value);
void D3DDevice_SetRenderState_StencilRef(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_TwoSidedStencilMode(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_ZEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_ZFunc(D3DDevice *pDevice, D3DCMPFUNC Value);
void D3DDevice_SetRenderState_ZWriteEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_PointSizeMax(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_SeparateAlphaBlendEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_BlendOpAlpha(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_PresentInterval(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_PresentImmediateThreshold(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_PointSize(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_ViewportEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_HalfPixelOffset(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_StencilMask(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_StencilWriteMask(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_CCWStencilFail(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_CCWStencilFunc(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_CCWStencilMask(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_CCWStencilPass(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_CCWStencilRef(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_CCWStencilWriteMask(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_CCWStencilZFail(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_ClipPlaneEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_PointSize(D3DDevice *pDevice, DWORD FloatAsDword);
void D3DDevice_SetRenderState_PointSizeMin(D3DDevice *pDevice, DWORD FloatAsDword);
void D3DDevice_SetRenderState_PointSizeMax(D3DDevice *pDevice, DWORD FloatAsDword);
void D3DDevice_SetRenderState_PointSpriteEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_MultiSampleAntiAlias(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_MultiSampleMask(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_ScissorTestEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_SlopeScaleDepthBias(D3DDevice *pDevice, DWORD FloatAsDword);
void D3DDevice_SetRenderState_DepthBias(D3DDevice *pDevice, DWORD FloatAsDword);
void D3DDevice_SetRenderState_ColorWriteEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_ColorWriteEnable1(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_ColorWriteEnable2(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_ColorWriteEnable3(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_TessellationMode(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_MinTessellationLevel(D3DDevice *pDevice, DWORD FloatAsDword);
void D3DDevice_SetRenderState_MaxTessellationLevel(D3DDevice *pDevice, DWORD FloatAsDword);
void D3DDevice_SetRenderState_Wrap0(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_Wrap1(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_Wrap2(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_Wrap3(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_Wrap4(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_Wrap5(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_Wrap6(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_Wrap7(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_Wrap8(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_Wrap9(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_Wrap10(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_Wrap11(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_Wrap12(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_Wrap13(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_Wrap14(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_Wrap15(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_HighPrecisionBlendEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_HighPrecisionBlendEnable1(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_HighPrecisionBlendEnable2(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_HighPrecisionBlendEnable3(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_PrimitiveResetEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_PrimitiveResetIndex(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_AlphaToMaskEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_AlphaToMaskOffsets(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_GuardBand_X(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_GuardBand_Y(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_DiscardBand_X(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_DiscardBand_Y(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_HiStencilEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_HiStencilWriteEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_HiStencilFunc(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_HiStencilRef(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_PresentInterval(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_PresentImmediateThreshold(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_HiZEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_HiZWriteEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_LastPixel(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_LineWidth(D3DDevice *pDevice, DWORD FloatAsDword);
void D3DDevice_SetRenderState_Buffer2Frames(D3DDevice *pDevice, DWORD Value);

void D3DDevice_SetRenderState(D3DDevice *pDevice, D3DRENDERSTATETYPE State, DWORD Value);

void D3DDevice_SetSamplerState_AddressU(D3DDevice *pDevice, DWORD Sampler, DWORD Value);
void D3DDevice_SetSamplerState_AddressV(D3DDevice *pDevice, DWORD Sampler, DWORD Value);
void D3DDevice_SetSamplerState_AddressW(D3DDevice *pDevice, DWORD Sampler, DWORD Value);
void D3DDevice_SetSamplerState_BorderColor(D3DDevice *pDevice, DWORD Sampler, DWORD Value);
void D3DDevice_SetSamplerState_MinFilter(D3DDevice *pDevice, DWORD Sampler, DWORD Value);
void D3DDevice_SetSamplerState_MagFilter(D3DDevice *pDevice, DWORD Sampler, DWORD Value);
void D3DDevice_SetSamplerState_MipFilter(D3DDevice *pDevice, DWORD Sampler, DWORD Value);
void D3DDevice_SetSamplerState_MipMapLodBias(
    D3DDevice *pDevice, DWORD Sampler, DWORD FloatAsDword
);
void D3DDevice_SetSamplerState_MaxMipLevel(D3DDevice *pDevice, DWORD Sampler, DWORD Value);

void D3DDevice_SetSamplerState_MaxAnisotropy(
    D3DDevice *pDevice, DWORD Sampler, DWORD Value
);
void D3DDevice_SetSamplerState_MagFilterZ(D3DDevice *pDevice, DWORD Sampler, DWORD Value);
void D3DDevice_SetSamplerState_MinFilterZ(D3DDevice *pDevice, DWORD Sampler, DWORD Value);
void D3DDevice_SetSamplerState_SeparateZFilterEnable(
    D3DDevice *pDevice, DWORD Sampler, DWORD Value
);
void D3DDevice_SetSamplerState_MinMipLevel(D3DDevice *pDevice, DWORD Sampler, DWORD Value);
void D3DDevice_SetSamplerState_TrilinearThreshold(
    D3DDevice *pDevice, DWORD Sampler, DWORD Value
);
void D3DDevice_SetSamplerState_AnisotropyBias(
    D3DDevice *pDevice, DWORD Sampler, DWORD FloatAsDword
);
void D3DDevice_SetSamplerState_HGradientExpBias(
    D3DDevice *pDevice, DWORD Sampler, DWORD Value
);
void D3DDevice_SetSamplerState_VGradientExpBias(
    D3DDevice *pDevice, DWORD Sampler, DWORD Value
);
void D3DDevice_SetSamplerState_WhiteBorderColorW(
    D3DDevice *pDevice, DWORD Sampler, DWORD Value
);
void D3DDevice_SetSamplerState_PointBorderEnable(
    D3DDevice *pDevice, DWORD Sampler, DWORD Value
);

void D3DDevice_SetSamplerState(
    D3DDevice *pDevice, DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value
);

DWORD D3DDevice_GetSamplerState_MinFilter(D3DDevice *pDevice, DWORD Sampler);
DWORD D3DDevice_GetSamplerState_MinFilterZ(D3DDevice *pDevice, DWORD Sampler);
DWORD D3DDevice_GetSamplerState_MagFilter(D3DDevice *pDevice, DWORD Sampler);
DWORD D3DDevice_GetSamplerState_MagFilterZ(D3DDevice *pDevice, DWORD Sampler);
DWORD D3DDevice_GetSamplerState_MipFilter(D3DDevice *pDevice, DWORD Sampler);
DWORD D3DDevice_GetSamplerState_SeparateZFilterEnable(D3DDevice *pDevice, DWORD Sampler);
DWORD D3DDevice_GetSamplerState_MaxAnisotropy(D3DDevice *pDevice, DWORD Sampler);
DWORD D3DDevice_GetSamplerState_AnisotropyBias(D3DDevice *pDevice, DWORD Sampler);
DWORD D3DDevice_GetSamplerState_MipMapLodBias(D3DDevice *pDevice, DWORD Sampler);
DWORD D3DDevice_GetSamplerState_MaxMipLevel(D3DDevice *pDevice, DWORD Sampler);
DWORD D3DDevice_GetSamplerState_MinMipLevel(D3DDevice *pDevice, DWORD Sampler);
DWORD D3DDevice_GetSamplerState_BorderColor(D3DDevice *pDevice, DWORD Sampler);
DWORD D3DDevice_GetSamplerState_AddressU(D3DDevice *pDevice, DWORD Sampler);
DWORD D3DDevice_GetSamplerState_AddressV(D3DDevice *pDevice, DWORD Sampler);
DWORD D3DDevice_GetSamplerState_AddressW(D3DDevice *pDevice, DWORD Sampler);
DWORD D3DDevice_GetSamplerState_TrilinearThreshold(D3DDevice *pDevice, DWORD Sampler);
DWORD D3DDevice_GetSamplerState_HGradientExpBias(D3DDevice *pDevice, DWORD Sampler);
DWORD D3DDevice_GetSamplerState_VGradientExpBias(D3DDevice *pDevice, DWORD Sampler);
DWORD D3DDevice_GetSamplerState_WhiteBorderColorW(D3DDevice *pDevice, DWORD Sampler);
DWORD D3DDevice_GetSamplerState_PointBorderEnable(D3DDevice *pDevice, DWORD Sampler);

DWORD D3DDevice_GetSamplerState(
    D3DDevice *pDevice, DWORD Sampler, D3DSAMPLERSTATETYPE Type
);

DWORD D3DDevice_InsertFence(D3DDevice *pDevice);
void D3DDevice_BlockOnFence(DWORD fence);
D3DVertexBuffer *D3DDevice_CreateVertexBuffer(UINT Length, DWORD Usage, UINT Pool);

D3DIndexBuffer *
D3DDevice_CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, UINT Pool);

HRESULT D3DDevice_SetStreamSource(
    D3DDevice *pDevice,
    UINT StreamNumber,
    D3DVertexBuffer *pVertexBuffer,
    UINT OffsetInBytes,
    UINT StrideInBytes,
    UINT64 PendingMask3
);

void D3DDevice_SetTexture(
    D3DDevice *pDevice, DWORD Sampler, D3DBaseTexture *pTexture, UINT64 PendingMask3
);
D3DSurface *D3DDevice_GetRenderTarget(D3DDevice *pDevice, DWORD RenderTargetIndex);
D3DBaseTexture *D3DDevice_CreateTexture(
    UINT Width,
    UINT Height,
    UINT Depth,
    UINT Levels,
    DWORD Usage,
    D3DFORMAT D3DFormat,
    UINT Pool,
    D3DRESOURCETYPE D3DType
);

D3DVertexDeclaration *
D3DDevice_CreateVertexDeclaration(const D3DVERTEXELEMENT9 *pVertexElements);

void D3DDevice_SetFVF(D3DDevice *pDevice, DWORD FVF);
void D3DDevice_DrawVerticesUP(
    D3DDevice *pDevice,
    D3DPRIMITIVETYPE PrimitiveType,
    UINT VertexCount,
    const void *pVertexStreamZeroData,
    UINT VertexStreamZeroStride
);
void D3DDevice_SetRenderTarget_External(
    D3DDevice *pDevice, UINT RenderTargetIndex, D3DSurface *pRenderTarget
);
void D3DDevice_SetDepthStencilSurface(D3DDevice *pDevice, D3DSurface *pZStencilSurface);
void D3DDevice_SetViewport(D3DDevice *pDevice, const D3DVIEWPORT9 *pViewport);
void D3DDevice_SetIndices(D3DDevice *pDevice, D3DIndexBuffer *pIndexData);
void D3DDevice_DrawIndexedVertices(
    D3DDevice *pDevice,
    D3DPRIMITIVETYPE PrimitiveType,
    INT BaseVertexIndex,
    UINT StartIndex,
    UINT IndexCount
);
HRESULT D3DDevice_Reset(D3DDevice *pDevice, D3DPRESENT_PARAMETERS *);
void D3DDevice_Clear(
    D3DDevice *pDevice,
    DWORD Count,
    const D3DRECT *pRects,
    DWORD Flags,
    DWORD Color,
    float Z,
    DWORD Stencil,
    BOOL EDRamClear
);

void D3DDevice_SynchronizeToPresentationInterval(D3DDevice *pDevice);
void D3DDevice_QuerySwapStatus(D3DDevice *pDevice, D3DSWAP_STATUS *pSwapStatus);
void D3DDevice_Swap(
    D3DDevice *pDevice,
    D3DBaseTexture *pFrontBuffer,
    const D3DVIDEO_SCALER_PARAMETERS *pParameters
);
void D3DDevice_BlockUntilIdle(D3DDevice *pDevice);
void D3DDevice_SetSwapMode(D3DDevice *pDevice, BOOL Asynchronous);
UINT D3DDevice_Release(D3DDevice *pDevice);
void D3DDevice_SetGammaRamp(
    D3DDevice *pDevice, DWORD UnusedFlags, const D3DGAMMARAMP *pRamp
);
void D3DDevice_BeginTiling(
    D3DDevice *pDevice,
    DWORD Flags,
    DWORD Count,
    const D3DRECT *pTileRects,
    const XMVECTOR *pClearColor,
    float ClearZ,
    DWORD ClearStencil
);
D3DPerfCounters *D3DDevice_CreatePerfCounters(D3DDevice *pDevice, UINT NumPasses);

HRESULT
D3DDevice_EndTiling(
    D3DDevice *pDevice,
    UINT ResolveFlags,
    const D3DRECT *pResolveRects,
    D3DBaseTexture *pDestTexture,
    const XMVECTOR *pClearColor,
    float ClearZ,
    UINT ClearStencil,
    const D3DRESOLVE_PARAMETERS *pParameters
);

void D3DDevice_Resolve(
    D3DDevice *pDevice,
    UINT Flags,
    const D3DRECT *pSourceRect,
    D3DBaseTexture *pDestTexture,
    const D3DPOINT *pDestPoint,
    UINT DestLevel,
    UINT DestSliceOrFace,
    const XMVECTOR *pClearColor,
    float ClearZ,
    UINT ClearStencil,
    const D3DRESOLVE_PARAMETERS *pParameters
);

void D3DDevice_Suspend(D3DDevice *pDevice);
void D3DDevice_Resume(D3DDevice *pDevice);
void D3DDevice_SetShaderGPRAllocation(
    D3DDevice *pDevice, DWORD Flags, DWORD VertexShaderCount, DWORD PixelShaderCount
);
D3DSurface *D3DDevice_GetDepthStencilSurface(D3DDevice *pDevice);

D3DQuery *
D3DDevice_CreateQueryTiled(D3DDevice *pDevice, D3DQUERYTYPE Type, UINT TileCapacity);

void D3DDevice_SetVertexShader(D3DDevice *pDevice, D3DVertexShader *pShader);
void D3DDevice_SetPixelShader(D3DDevice *pDevice, D3DPixelShader *pShader);
static void D3DDevice_SetVertexShaderConstantF(
    D3DDevice *pDevice, UINT StartRegister, const float *pConstantData, UINT Vector4fCount
);

void D3DDevice_SetVertexShaderConstantFN(
    D3DDevice *pDevice,
    UINT StartRegister,
    const float *pConstantData,
    UINT Vector4fCount,
    UINT64 PendingMask0
);

void D3DDevice_GetVertexShaderConstantF(
    D3DDevice *pDevice, UINT StartRegister, float *pConstantData, UINT Vector4fCount
);

void D3DDevice_SetVertexShaderConstantI(
    D3DDevice *pDevice, UINT StartRegister, const int *pConstantData, UINT Vector4iCount
);

void D3DDevice_SetVertexShaderConstantB(
    D3DDevice *pDevice, UINT StartRegister, const BOOL *pConstantData, UINT BoolCount
);

static void D3DDevice_SetPixelShaderConstantF(
    D3DDevice *pDevice, UINT StartRegister, const float *pConstantData, UINT Vector4fCount
);
static void D3DDevice_SetPixelShaderConstantF1(
    D3DDevice *__restrict pDevice,
    UINT StartRegister,
    const float *__restrict pConstantData
);
void D3DDevice_SetPixelShaderConstantFN(
    D3DDevice *pDevice,
    UINT StartRegister,
    const float *pConstantData,
    UINT Vector4fCount,
    UINT64 PendingMask1
);
void D3DDevice_SetPixelShaderConstantI(
    D3DDevice *pDevice, UINT StartRegister, const int *pConstantData, UINT Vector4iCount
);
void D3DDevice_SetPixelShaderConstantB(
    D3DDevice *pDevice, UINT StartRegister, const BOOL *pConstantData, UINT BoolCount
);

void D3DDevice_SetPredication(D3DDevice *pDevice, UINT TilePredication);

void *D3DDevice_BeginVertices(
    D3DDevice *pDevice,
    D3DPRIMITIVETYPE PrimitiveType,
    UINT NumVertices,
    UINT VertexStreamZeroStride
);

void D3DDevice_EndVertices(D3DDevice *pDevice);

HRESULT D3DDevice_BeginIndexedVertices(
    D3DDevice *pDevice,
    D3DPRIMITIVETYPE PrimitiveType,
    INT BaseVertexIndex,
    UINT NumVertices,
    UINT IndexCount,
    D3DFORMAT IndexDataFormat,
    UINT VertexStreamZeroStride,
    void **ppIndexData,
    void **
);

void D3DDevice_EndIndexedVertices(D3DDevice *pDevice);

void D3DDevice_SetVertexDeclaration(
    D3DDevice *pDevice, D3DVertexDeclaration *pDeclaration
);

void D3DDevice_SetClipPlane(D3DDevice *pDevice, DWORD Index, const float *pPlane);

HRESULT PIXCaptureGpuFrame(char *pFilename);

// C++
struct D3DDevice { /* Size=0x2b00 */
    /* 0x0000 */ _D3DTAGCOLLECTION m_Pending;
    /* 0x0028 */ UINT64 m_Predicated_PendingMask2;
    /* 0x0030 */ UINT *m_pRing;
    /* 0x0034 */ UINT *m_pRingLimit;
    /* 0x0038 */ UINT *m_pRingGuarantee;
    /* 0x003c */ UINT m_ReferenceCount;
    /* 0x0040 */ VOID (*m_SetRenderStateCall[101])(D3DDevice *, UINT);
    /* 0x01d4 */ VOID (*m_SetSamplerStateCall[20])(D3DDevice *, UINT, UINT);
    /* 0x0224 */ UINT (*m_GetRenderStateCall[101])(D3DDevice *);
    /* 0x03b8 */ UINT (*m_GetSamplerStateCall[20])(D3DDevice *, UINT);
    int filler[30]; // not part of the struct but i needed it to align
    /* 0x0480 */ D3DConstants m_Constants;
    /* 0x2820 */ float m_ClipPlanes[6][4];
    /* 0x2880 */ GPU_DESTINATIONPACKET m_DestinationPacket;
    /* 0x28c0 */ GPU_WINDOWPACKET m_WindowPacket;
    /* 0x28cc */ GPU_VALUESPACKET m_ValuesPacket;
    /* 0x2920 */ GPU_PROGRAMPACKET m_ProgramPacket;
    /* 0x2934 */ GPU_CONTROLPACKET m_ControlPacket;
    /* 0x2964 */ GPU_TESSELLATORPACKET m_TessellatorPacket;
    /* 0x29b8 */ GPU_MISCPACKET m_MiscPacket;
    /* 0x2a50 */ GPU_POINTPACKET m_PointPacket;
    /* 0x2a70 */ BYTE m_MaxAnisotropy[26];
    /* 0x2a8a */ BYTE m_ZFilter[26];

    ULONG AddRef();
    ULONG Release();
    HRESULT GetDirect3D(Direct3D **ppD3D9);
    HRESULT GetDeviceCaps(D3DCAPS9 *pCaps);
    HRESULT GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE *pMode);
    HRESULT GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters);
    HRESULT Reset(D3DPRESENT_PARAMETERS *pPresentationParameters) {
        return D3DDevice_Reset(this, pPresentationParameters);
    }
    HRESULT Present(
        const RECT *pSourceRect,
        const RECT *pDestRect,
        void *hDestWindowOverride,
        void *pDirtyRegion
    );
    HRESULT GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS *pRasterStatus);
    void SetGammaRamp(UINT iSwapChain, DWORD Flags, const D3DGAMMARAMP *pRamp) {
        D3DDevice_SetGammaRamp(this, Flags, pRamp);
    }
    void GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP *pRamp);
    void SetPWLGamma(UINT, const D3DPWLGAMMA *);
    void GetPWLGamma(D3DPWLGAMMA *);
    HRESULT CreateTexture(
        UINT Width,
        UINT Height,
        UINT Levels,
        DWORD Usage,
        D3DFORMAT Format,
        UINT Pool,
        D3DTexture **ppTexture,
        HANDLE *pSharedHandle
    ) {
        *ppTexture = (D3DTexture *)D3DDevice_CreateTexture(
            Width, Height, 1, Levels, Usage, Format, Pool, D3DRTYPE_TEXTURE
        );
        return *ppTexture ? S_OK : E_OUTOFMEMORY;
    }
    HRESULT CreateVolumeTexture(
        UINT Width,
        UINT Height,
        UINT Depth,
        UINT Levels,
        DWORD Usage,
        D3DFORMAT Format,
        UINT Pool,
        D3DVolumeTexture **ppVolumeTexture,
        HANDLE *pSharedHandle
    );
    HRESULT CreateCubeTexture(
        UINT EdgeLength,
        UINT Levels,
        DWORD Usage,
        D3DFORMAT Format,
        UINT Pool,
        D3DCubeTexture **ppCubeTexture,
        HANDLE *pSharedHandle
    ) {
        *ppCubeTexture = (D3DCubeTexture *)D3DDevice_CreateTexture(
            EdgeLength, EdgeLength, 6, Levels, Usage, Format, Pool, D3DRTYPE_CUBETEXTURE
        );
        return *ppCubeTexture ? S_OK : E_OUTOFMEMORY;
    }
    HRESULT CreateArrayTexture(
        UINT,
        UINT,
        UINT,
        UINT,
        DWORD Usage,
        D3DFORMAT Format,
        UINT Pool,
        D3DArrayTexture **ppArrayTexture,
        HANDLE *pSharedHandle
    );
    HRESULT CreateLineTexture(
        UINT,
        UINT,
        DWORD Usage,
        D3DFORMAT Format,
        UINT Pool,
        D3DLineTexture **ppLineTexture,
        HANDLE *pSharedHandle
    );
    HRESULT CreateVertexBuffer(
        UINT Length,
        DWORD Usage,
        DWORD FVF,
        UINT Pool,
        D3DVertexBuffer **ppVertexBuffer,
        HANDLE *pSharedHandle
    ) {
        *ppVertexBuffer = D3DDevice_CreateVertexBuffer(Length, Usage, Pool);
        return *ppVertexBuffer ? S_OK : E_OUTOFMEMORY;
    }
    HRESULT CreateIndexBuffer(
        UINT Length,
        DWORD Usage,
        D3DFORMAT Format,
        UINT Pool,
        D3DIndexBuffer **ppIndexBuffer,
        HANDLE *pSharedHandle
    ) {
        *ppIndexBuffer = D3DDevice_CreateIndexBuffer(Length, Usage, Format, Pool);
        return *ppIndexBuffer ? S_OK : E_OUTOFMEMORY;
    }
    HRESULT CreateRenderTarget(
        UINT Width,
        UINT Height,
        D3DFORMAT Format,
        D3DMULTISAMPLE_TYPE MultiSample,
        DWORD MultisampleQuality,
        BOOL Lockable,
        D3DSurface **ppSurface,
        const D3DSURFACE_PARAMETERS *
    );
    HRESULT CreateDepthStencilSurface(
        UINT Width,
        UINT Height,
        D3DFORMAT Format,
        D3DMULTISAMPLE_TYPE MultiSample,
        DWORD MultisampleQuality,
        BOOL Lockable,
        D3DSurface **ppSurface,
        const D3DSURFACE_PARAMETERS *
    );
    HRESULT SetRenderTarget(DWORD RenderTargetIndex, D3DSurface *pRenderTarget) {
        D3DDevice_SetRenderTarget_External(this, RenderTargetIndex, pRenderTarget);
        return S_OK;
    }
    HRESULT GetRenderTarget(DWORD RenderTargetIndex, D3DSurface **ppRenderTarget) {
        *ppRenderTarget = D3DDevice_GetRenderTarget(this, RenderTargetIndex);
        return *ppRenderTarget ? S_OK : E_OUTOFMEMORY;
    }
    HRESULT SetDepthStencilSurface(D3DSurface *pNewZStencil) {
        D3DDevice_SetDepthStencilSurface(this, pNewZStencil);
        return S_OK;
    }
    HRESULT GetDepthStencilSurface(D3DSurface **ppZStencilSurface) {
        *ppZStencilSurface = D3DDevice_GetDepthStencilSurface(this);
        return *ppZStencilSurface ? S_OK : E_OUTOFMEMORY;
    }
    HRESULT
    GetBackBuffer(
        UINT iSwapChain, UINT iBackBuffer, UINT BackBufferType, D3DSurface **ppBackBuffer
    );
    HRESULT GetFrontBuffer(D3DTexture **);
    HRESULT BeginScene();
    HRESULT EndScene();
    HRESULT Clear(
        DWORD Count, const D3DRECT *pRects, DWORD Flags, DWORD Color, float Z, DWORD Stencil
    ) {
        D3DDevice_Clear(this, Count, pRects, Flags, Color, Z, Stencil, false);
        return S_OK;
    }
    HRESULT ClearF(DWORD, const D3DRECT *, const XMVECTOR *, float, DWORD);
    HRESULT SetViewport(const D3DVIEWPORT9 *pViewport) {
        D3DDevice_SetViewport(this, pViewport);
        return S_OK;
    }
    HRESULT GetViewport(D3DVIEWPORT9 *pViewport);
    __forceinline HRESULT SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) {
        D3DDevice_SetRenderState(this, State, Value);
        return S_OK;
    }
    HRESULT SetRenderState_Inline(D3DRENDERSTATETYPE State, DWORD Value);
    HRESULT GetRenderState(D3DRENDERSTATETYPE State, DWORD *pValue);
    HRESULT CreateStateBlock(D3DSTATEBLOCKTYPE Type, D3DStateBlock **ppSB);
    HRESULT GetTexture(DWORD Stage, D3DBaseTexture **ppTexture);
    HRESULT SetTexture(DWORD Stage, D3DBaseTexture *pTexture) {
        D3DDevice_SetTexture(this, Stage, pTexture, 0x8000000000000000 >> (Stage + 0x20U));
        return S_OK;
    }
    __forceinline HRESULT
    GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD *pValue) {
        *pValue = D3DDevice_GetSamplerState(this, Sampler, Type);
        return *pValue ? S_OK : E_OUTOFMEMORY;
    }
    __forceinline HRESULT
    SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {
        D3DDevice_SetSamplerState(this, Sampler, Type, Value);
        return S_OK;
    }
    HRESULT SetSamplerState_Inline(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
    HRESULT SetSamplerAddressStates(DWORD, DWORD, DWORD, DWORD);
    HRESULT SetSamplerBorderStates(DWORD, DWORD, DWORD, DWORD);
    HRESULT
    SetSamplerSeparateZFilterStates(DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD);
    HRESULT SetSamplerFilterStates(DWORD, DWORD, DWORD, DWORD, DWORD);
    HRESULT
    DrawVertices(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT VertexCount);
    HRESULT DrawIndexedVertices(
        D3DPRIMITIVETYPE PrimitiveType,
        INT BaseVertexIndex,
        UINT StartIndex,
        UINT IndexCount
    ) {
        D3DDevice_DrawIndexedVertices(
            this, PrimitiveType, BaseVertexIndex, StartIndex, IndexCount
        );
        return S_OK;
    }
    HRESULT
    DrawVerticesUP(
        D3DPRIMITIVETYPE PrimitiveType,
        UINT VertexCount,
        const void *pVertexStreamZeroData,
        UINT VertexStreamZeroStride
    ) {
        D3DDevice_DrawVerticesUP(
            this, PrimitiveType, VertexCount, pVertexStreamZeroData, VertexStreamZeroStride
        );
        return S_OK;
    }
    HRESULT DrawIndexedVerticesUP(
        D3DPRIMITIVETYPE PrimitiveType,
        UINT MinVertexIndex,
        UINT NumVertices,
        UINT PrimitiveCount,
        const void *pIndexData,
        D3DFORMAT IndexDataFormat,
        const void *pVertexData,
        UINT VertexStride
    );
    HRESULT
    DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
    HRESULT DrawIndexedPrimitive(
        D3DPRIMITIVETYPE PrimitiveType,
        INT BaseVertexIndex,
        UINT MinVertexIndex,
        UINT NumVertices,
        UINT startIndex,
        UINT primCount
    );
    HRESULT DrawPrimitiveUP(
        D3DPRIMITIVETYPE PrimitiveType,
        UINT PrimitiveCount,
        const void *pVertexStreamZeroData,
        UINT VertexStreamZeroStride
    );
    HRESULT DrawIndexedPrimitiveUP(
        D3DPRIMITIVETYPE PrimitiveType,
        UINT MinVertexIndex,
        UINT NumVertices,
        UINT PrimitiveCount,
        const void *pIndexData,
        D3DFORMAT IndexDataFormat,
        const void *pVertexStreamZeroData,
        UINT VertexStreamZeroStride
    );
    HRESULT SetFVF(DWORD FVF) {
        D3DDevice_SetFVF(this, FVF);
        return S_OK;
    }
    HRESULT GetFVF(DWORD *pFVF);
    HRESULT CreateVertexShader(const DWORD *pFunction, D3DVertexShader **ppShader) {
        *ppShader = D3DDevice_CreateVertexShader(pFunction);
        return *ppShader ? S_OK : E_OUTOFMEMORY;
    }
    HRESULT SetVertexShader(D3DVertexShader *pShader) {
        D3DDevice_SetVertexShader(this, pShader);
        return S_OK;
    }
    HRESULT GetVertexShader(D3DVertexShader **ppShader);
    HRESULT SetVertexShaderConstantB(
        UINT StartRegister, const BOOL *pConstantData, UINT BoolCount
    ) {
        D3DDevice_SetVertexShaderConstantB(this, StartRegister, pConstantData, BoolCount);
        return S_OK;
    }
    __forceinline HRESULT SetVertexShaderConstantF(
        UINT StartRegister, const float *pConstantData, UINT Vector4fCount
    ) {
        D3DDevice_SetVertexShaderConstantF(
            this, StartRegister, pConstantData, Vector4fCount
        );
        return S_OK;
    }
    HRESULT SetVertexShaderConstantI(
        UINT StartRegister, const int *pConstantData, UINT Vector4iCount
    ) {
        D3DDevice_SetVertexShaderConstantI(
            this, StartRegister, pConstantData, Vector4iCount
        );
        return S_OK;
    }
    HRESULT
    GetVertexShaderConstantB(UINT StartRegister, BOOL *pConstantData, UINT BoolCount);
    HRESULT
    GetVertexShaderConstantF(UINT StartRegister, float *pConstantData, UINT Vector4fCount);
    HRESULT
    GetVertexShaderConstantI(UINT StartRegister, int *pConstantData, UINT Vector4iCount);
    HRESULT SetVertexShaderConstantF_NotInline(
        UINT StartRegister, const float *pConstantData, UINT Vector4fCount
    );
    HRESULT SetStreamSource(
        UINT StreamNumber, D3DVertexBuffer *pStreamData, UINT OffsetInBytes, UINT Stride
    ) {
        D3DDevice_SetStreamSource(
            this, StreamNumber, pStreamData, OffsetInBytes, Stride, 1
        );
        return S_OK;
    }
    HRESULT GetStreamSource(
        UINT StreamNumber,
        D3DVertexBuffer **ppStreamData,
        UINT *OffsetInBytes,
        UINT *pStride
    );
    HRESULT SetIndices(D3DIndexBuffer *pIndexData) {
        D3DDevice_SetIndices(this, pIndexData);
        return S_OK;
    }
    HRESULT GetIndices(D3DIndexBuffer **ppIndexData);
    HRESULT CreatePixelShader(const DWORD *pFunction, D3DPixelShader **ppShader) {
        *ppShader = D3DDevice_CreatePixelShader(pFunction);
        return *ppShader ? S_OK : E_OUTOFMEMORY;
    }
    HRESULT SetPixelShader(D3DPixelShader *pShader) {
        D3DDevice_SetPixelShader(this, pShader);
        return S_OK;
    }
    HRESULT GetPixelShader(D3DPixelShader **ppShader);
    HRESULT
    SetPixelShaderConstantB(
        UINT StartRegister, const BOOL *pConstantData, UINT BoolCount
    ) {
        D3DDevice_SetPixelShaderConstantB(this, StartRegister, pConstantData, BoolCount);
        return S_OK;
    }
    __forceinline HRESULT SetPixelShaderConstantF(
        UINT StartRegister, const float *pConstantData, UINT Vector4fCount
    ) {
        D3DDevice_SetPixelShaderConstantF(
            this, StartRegister, pConstantData, Vector4fCount
        );
        return S_OK;
    }
    HRESULT SetPixelShaderConstantI(
        UINT StartRegister, const int *pConstantData, UINT Vector4iCount
    ) {
        D3DDevice_SetPixelShaderConstantI(
            this, StartRegister, pConstantData, Vector4iCount
        );
        return S_OK;
    }
    HRESULT
    GetPixelShaderConstantB(UINT StartRegister, BOOL *pConstantData, UINT BoolCount);
    HRESULT
    GetPixelShaderConstantF(UINT StartRegister, float *pConstantData, UINT Vector4fCount);
    HRESULT
    GetPixelShaderConstantI(UINT StartRegister, int *pConstantData, UINT Vector4iCount);
    HRESULT SetPixelShaderConstantF_NotInline(
        UINT StartRegister, const float *pConstantData, UINT Vector4fCount
    );
    HRESULT CreateVertexDeclaration(
        const D3DVERTEXELEMENT9 *pVertexElements, D3DVertexDeclaration **ppDecl
    ) {
        *ppDecl = D3DDevice_CreateVertexDeclaration(pVertexElements);
        return *ppDecl ? S_OK : E_OUTOFMEMORY;
    }
    HRESULT SetVertexDeclaration(D3DVertexDeclaration *pDecl) {
        D3DDevice_SetVertexDeclaration(this, pDecl);
        return S_OK;
    }
    HRESULT GetVertexDeclaration(D3DVertexDeclaration **ppDecl);
    HRESULT SetScissorRect(const RECT *pRect);
    HRESULT GetScissorRect(RECT *pRect);
    HRESULT SetClipPlane(DWORD Index, const float *pPlane) {
        D3DDevice_SetClipPlane(this, Index, pPlane);
        return S_OK;
    }
    HRESULT GetClipPlane(DWORD Index, float *pPlane);
    HRESULT CreateQuery(D3DQUERYTYPE Type, D3DQuery **ppQuery);
    HRESULT CreateQueryTiled(D3DQUERYTYPE Type, UINT TileCapacity, D3DQuery **ppQuery) {
        *ppQuery = D3DDevice_CreateQueryTiled(this, Type, TileCapacity);
        return *ppQuery ? S_OK : E_OUTOFMEMORY;
    }
    HRESULT Resolve(
        DWORD Flags,
        const D3DRECT *pSourceRect,
        D3DBaseTexture *pDestTexture,
        const D3DPOINT *pDestPoint,
        UINT DestLevel,
        UINT DestSliceOrFace,
        const XMVECTOR *pClearColor,
        float ClearZ,
        UINT ClearStencil,
        const D3DRESOLVE_PARAMETERS *pParameters
    ) {
        D3DDevice_Resolve(
            this,
            Flags,
            pSourceRect,
            pDestTexture,
            pDestPoint,
            DestLevel,
            DestSliceOrFace,
            pClearColor,
            ClearZ,
            ClearStencil,
            pParameters
        );
        return S_OK;
    }
    HRESULT AcquireThreadOwnership();
    HRESULT ReleaseThreadOwnership();
    HRESULT SetThreadOwnership(DWORD);
    DWORD QueryThreadOwnership();
    BOOL IsBusy();
    HRESULT BlockUntilIdle() {
        D3DDevice_BlockUntilIdle(this);
        return S_OK;
    }
    HRESULT InsertCallback(UINT Type, D3DCALLBACK pCallback, UINT Context);
    HRESULT SetVerticalBlankCallback(D3DVBLANKCALLBACK pCallback);
    HRESULT SetSwapCallback(D3DSWAPCALLBACK pCallback);
    HRESULT SynchronizeToPresentationInterval() {
        D3DDevice_SynchronizeToPresentationInterval(this);
        return S_OK;
    }
    HRESULT
    Swap(D3DBaseTexture *pFrontBuffer, const D3DVIDEO_SCALER_PARAMETERS *pParameters) {
        D3DDevice_Swap(this, pFrontBuffer, pParameters);
        return S_OK;
    }
    HRESULT RenderSystemUI();
    HRESULT QueryBufferSpace(UINT *, UINT *);
    HRESULT SetPredication(UINT TilePredication) {
        D3DDevice_SetPredication(this, TilePredication);
        return S_OK;
    }
    HRESULT SetPatchablePredication(UINT, UINT);
    HRESULT BeginTiling(
        DWORD Flags,
        DWORD Count,
        const D3DRECT *pTileRects,
        const XMVECTOR *pClearColor,
        float ClearZ,
        DWORD ClearStencil
    ) {
        D3DDevice_BeginTiling(
            this, Flags, Count, pTileRects, pClearColor, ClearZ, ClearStencil
        );
        return S_OK;
    }
    HRESULT EndTiling(
        UINT ResolveFlags,
        const D3DRECT *pResolveRects,
        D3DBaseTexture *pDestTexture,
        const XMVECTOR *pClearColor,
        float ClearZ,
        UINT ClearStencil,
        const D3DRESOLVE_PARAMETERS *pParameters
    ) {
        return D3DDevice_EndTiling(
            this,
            ResolveFlags,
            pResolveRects,
            pDestTexture,
            pClearColor,
            ClearZ,
            ClearStencil,
            pParameters
        );
    }
    HRESULT BeginZPass(UINT);
    HRESULT EndZPass();
    HRESULT InvokeRenderPass();
    HRESULT BeginExport(UINT, D3DResource *, UINT);
    HRESULT EndExport(UINT, D3DResource *, UINT);
    HRESULT DrawTessellatedPrimitive(D3DTESSPRIMITIVETYPE, UINT, UINT);
    HRESULT DrawIndexedTessellatedPrimitive(D3DTESSPRIMITIVETYPE, INT, UINT, UINT);
    HRESULT SetRingBufferParameters(const D3DRING_BUFFER_PARAMETERS *);

    //   HRESULT XpsBegin(uint32_t);
    //   HRESULT XpsEnd();
    //   HRESULT XpsSetCallback(void (*)(D3DXpsThread*, void*, const void*, uint32_t),
    //      void*, uint32_t);
    //   HRESULT XpsSubmit(uint32_t, const void*, uint32_t);
    //   HRESULT XpsSetPredication(uint32_t);
    //   HRESULT XpsSetPredicationFromVisibility(void*);
    //   HRESULT XpsSetPredicationFromQuery(D3DQuery*);

    HRESULT BeginVertices(
        D3DPRIMITIVETYPE PrimitiveType,
        UINT VertexCount,
        UINT VertexStreamZeroStride,
        void **out
    ) {
        *out = D3DDevice_BeginVertices(
            this, PrimitiveType, VertexCount, VertexStreamZeroStride
        );
        return *out ? S_OK : E_OUTOFMEMORY;
    }
    HRESULT EndVertices() {
        D3DDevice_EndVertices(this);
        return S_OK;
    }
    HRESULT BeginIndexedVertices(
        D3DPRIMITIVETYPE PrimitiveType,
        INT BaseVertexIndex,
        UINT NumVertices,
        UINT IndexCount,
        D3DFORMAT IndexDataFormat,
        UINT VertexStreamZeroStride,
        void **ppIndexData,
        void **v
    ) {
        return D3DDevice_BeginIndexedVertices(
            this,
            PrimitiveType,
            BaseVertexIndex,
            NumVertices,
            IndexCount,
            IndexDataFormat,
            VertexStreamZeroStride,
            ppIndexData,
            v
        );
    }
    HRESULT EndIndexedVertices() {
        D3DDevice_EndIndexedVertices(this);
        return S_OK;
    }
    DWORD InsertFence() { return D3DDevice_InsertFence(this); }
    HRESULT BlockOnFence(DWORD fence) {
        D3DDevice_BlockOnFence(fence);
        return S_OK;
    }

    //   int32_t IsFencePending(uint32_t);
    //   int32_t SetBlendState(uint32_t, _D3DBLENDSTATE);
    //   int32_t GetBlendState(uint32_t, _D3DBLENDSTATE*);
    //   int32_t SetVertexFetchConstant(uint32_t, D3DVertexBuffer*, uint32_t);
    //   int32_t SetTextureFetchConstant(uint32_t, D3DBaseTexture*);
    //   float GetCounter(_D3DCOUNTER);
    //   int32_t SetSafeLevel(uint32_t, uint32_t);
    //   int32_t GetSafeLevel(uint32_t*, uint32_t*);
    //   int32_t SetHangCallback(void (*)(const char*));
    //   int32_t BeginConditionalSurvey(uint32_t, uint32_t);
    //   int32_t EndConditionalSurvey(uint32_t);
    //   int32_t BeginConditionalRendering(uint32_t);
    //   int32_t EndConditionalRendering();
    //   int32_t PersistDisplay(D3DTexture*, const _D3DVIDEO_SCALER_PARAMETERS*);
    //   int32_t GetPersistedTexture(D3DTexture**);

    HRESULT Suspend() {
        D3DDevice_Suspend(this);
        return S_OK;
    }
    HRESULT Resume() {
        D3DDevice_Resume(this);
        return S_OK;
    }
    HRESULT CreatePerfCounters(D3DPerfCounters **ppPerfCounters, UINT NumPasses) {
        *ppPerfCounters = D3DDevice_CreatePerfCounters(this, NumPasses);
        return *ppPerfCounters ? S_OK : E_OUTOFMEMORY;
    }
    HRESULT EnablePerfCounters(BOOL Enable) {
        D3DDevice_EnablePerfCounters(this, Enable);
        return S_OK;
    }
    HRESULT SetPerfCounterEvents(const D3DPERFCOUNTER_EVENTS *pEvents, DWORD Flags) {
        D3DDevice_SetPerfCounterEvents(this, pEvents, Flags);
        return S_OK;
    }
    HRESULT QueryPerfCounters(D3DPerfCounters *pCounters, DWORD Flags) {
        D3DDevice_QueryPerfCounters(this, pCounters, Flags);
        return S_OK;
    }
    UINT GetNumPasses();
    HRESULT SetShaderInstructionAllocation(DWORD, DWORD, DWORD);
    HRESULT
    SetShaderGPRAllocation(DWORD Flags, DWORD VertexShaderCount, DWORD PixelShaderCount) {
        D3DDevice_SetShaderGPRAllocation(this, Flags, VertexShaderCount, PixelShaderCount);
        return S_OK;
    }
    HRESULT GetShaderGPRAllocation(DWORD *, DWORD *, DWORD *);
    HRESULT SetScreenExtentQueryMode(D3DSCREENEXTENTQUERYMODE);
    HRESULT GetScreenExtentQueryMode(D3DSCREENEXTENTQUERYMODE *);

    //   int32_t BeginPixelShaderConstantF1(uint32_t, __vector4**, uint32_t);
    //   int32_t EndPixelShaderConstantF1();
    //   int32_t BeginVertexShaderConstantF1(uint32_t, __vector4**, uint32_t);
    //   int32_t EndVertexShaderConstantF1();
    //   int32_t BeginPixelShaderConstantF4(uint32_t, __vector4**, __vector4**, uint32_t);
    //   int32_t EndPixelShaderConstantF4();
    //   int32_t BeginVertexShaderConstantF4(uint32_t,
    //      __vector4**, __vector4**, uint32_t);
    //   int32_t EndVertexShaderConstantF4();
    //   uint32_t GetCurrentFence();
    //   int32_t InvalidateGpuCache(void*, uint32_t, uint32_t);
    //   int32_t InvalidateResourceGpuCache(D3DResource*, uint32_t);
    //   int32_t FlushHiZStencil(_D3DFHZS_FLUSHTYPE);
    //   int32_t UnsetAll();
    //   uint32_t GetDeviceState();
    //   int32_t SetBlockCallback(uint32_t,
    //      void (*)(uint32_t, _D3DBLOCKTYPE, float, uint32_t));
    //   int32_t SetSurfaces(const _D3DSURFACES*, uint32_t);
    //   int32_t CreateConstantBuffer(uint32_t, uint32_t, D3DConstantBuffer**);
    //   int32_t CreateCommandBuffer(uint32_t, uint32_t, D3DCommandBuffer**);
    //   int32_t CreateGrowableCommandBuffer(uint32_t,
    //      void* (*)(uint32_t, uint32_t, uint32_t*, uint32_t),
    //      void (*)(uint32_t), void (*)(uint32_t, uint32_t*, uint32_t*),
    //      uint32_t, uint32_t, D3DCommandBuffer**);
    //   int32_t BeginCommandBuffer(D3DCommandBuffer*, uint32_t,
    //      const _D3DTAGCOLLECTION*, const _D3DTAGCOLLECTION*,
    //      const _D3DRECT*, uint32_t);
    //   int32_t EndCommandBuffer();
    //   int32_t RunCommandBuffer(D3DCommandBuffer*, uint32_t);
    //   int32_t InsertAsyncCommandBufferCall(D3DAsyncCommandBufferCall*,
    //      uint32_t, uint32_t);
    //   int32_t SetCommandBufferPredication(uint32_t, uint32_t);
    //   uint32_t InsertMarker();
    //   int32_t Nop(uint32_t);

    HRESULT QuerySwapStatus(D3DSWAP_STATUS *pSwapStatus) {
        D3DDevice_QuerySwapStatus(this, pSwapStatus);
        return S_OK;
    }

    //   uint32_t PixBeginNamedEvent(uint32_t, const char*, ...);
    //   uint32_t PixEndNamedEvent();
    //   void PixSetMarker(uint32_t, const char*, ...);
    //   void PixIgnoreTexture(D3DBaseTexture*);
    //   void PixStopIgnoringTexture(D3DBaseTexture*);
    //   void PixIgnoreMemoryRange(const void*, uint32_t);
    //   void PixStopIgnoringMemoryRange(const void*, uint32_t);
    //   int32_t PixSetTextureName(D3DBaseTexture*, const char*);
    //   void PixReportNewTexture(D3DBaseTexture*);
    //   void PixReportDeletedTexture(D3DBaseTexture*, int32_t, int32_t);
    //   void PixReportMovedMemoryRange(const void*, const void*, uint32_t);
    //   void PixReportFreedMemoryRange(const void*, uint32_t);
    //   int32_t SetViewportF(const _D3DVIEWPORTF9*);
    //   int32_t GetViewportF(_D3DVIEWPORTF9*);
    //   void* BeginVisibilitySurvey(uint32_t);
    //   int32_t EndVisibilitySurvey(void*);

    HRESULT SetSwapMode(BOOL Asynchronous) {
        D3DDevice_SetSwapMode(this, Asynchronous);
        return S_OK;
    }

    // clang-format off
//   uint64_t InsertBlockOnAsyncResources(uint32_t, D3DResource**, uint32_t, D3DResource**, uint32_t);
//   int32_t SignalAsyncResources(uint64_t);
//   int32_t CreateAsyncCommandBufferCall(_D3DTAGCOLLECTION*, _D3DTAGCOLLECTION*, uint32_t, uint32_t, D3DAsyncCommandBufferCall**);
//   int32_t CreatePerfCounterBatch(uint32_t, uint32_t, uint32_t, D3DPerfCounterBatch**);
//   int32_t CreateOcclusionQueryBatch(uint32_t, uint32_t, D3DOcclusionQueryBatch**);
//   void GpuOwn(_D3DTAG);
//   void GpuOwnSubset(_D3DTAG, uint32_t, uint32_t);
//   void GpuOwnVertexShaderConstantF(uint32_t, uint32_t);
//   void GpuOwnPixelShaderConstantF(uint32_t, uint32_t);
//   void GpuOwnShaders();
//   void GpuDisown(_D3DTAG);
//   void GpuDisownSubset(_D3DTAG, uint32_t, uint32_t);
//   void GpuDisownVertexShaderConstantF(uint32_t, uint32_t);
//   void GpuDisownPixelShaderConstantF(uint32_t, uint32_t);
//   void GpuDisownShaders();
//   void GpuDisownAll();
//   void GpuLoadPixelShaderConstantF4(uint32_t, uint32_t, D3DConstantBuffer*, uint32_t);
//   void GpuLoadVertexShaderConstantF4(uint32_t, uint32_t, D3DConstantBuffer*, uint32_t);
//   void GpuLoadPixelShaderConstantF4Pointer(uint32_t, const void*, uint32_t);
//   void GpuLoadVertexShaderConstantF4Pointer(uint32_t, const void*, uint32_t);
//   int32_t GpuBeginVertexShaderConstantF4(uint32_t, __vector4**, uint32_t);
//   void GpuEndVertexShaderConstantF4();
//   int32_t GpuBeginPixelShaderConstantF4(uint32_t, __vector4**, uint32_t);
//   void GpuEndPixelShaderConstantF4();
//   void GpuLoadShaders(D3DVertexShader*, D3DPixelShader*, const GPUFLOW_CONSTANTS*);
//   void GpuLoadShadersFast(D3DVertexShader*, D3DPixelShader*, const GPUFLOW_CONSTANTS*, uint32_t);
//   void GpuSetVertexFetchConstant(uint32_t, D3DVertexBuffer*, uint32_t, D3DVertexBuffer*, uint32_t, D3DVertexBuffer*, uint32_t);
//   void GpuSetTextureFetchConstant(uint32_t, D3DBaseTexture*);
//   void GpuSetColorMask(const GPU_COLORMASK*);
//   void GpuSetBlendFactor(const _D3DCOLORVALUE*);
//   void GpuSetAlphaRef(float);
//   void GpuSetBlendControl(uint32_t, const GPU_BLENDCONTROL*);
//   void GpuSetColorControl(const GPU_COLORCONTROL*);
    // clang-format on
};

static inline void D3DDevice_SetVertexShaderConstantF1(
    D3DDevice *pDevice, UINT StartRegister, const float *__restrict pConstantData
) {
    float f1 = pConstantData[0];
    float f2 = pConstantData[1];
    float f3 = pConstantData[2];
    float f4 = pConstantData[3];
    pDevice->m_Pending.m_Mask[0] |= (0x8000000000000000U >> (StartRegister >> 2));
    float *fDest = (float *)&pDevice->m_Constants.VertexShaderF[StartRegister];
    fDest[0] = f1;
    fDest[1] = f2;
    fDest[2] = f3;
    fDest[3] = f4;
    // XMVECTOR &vec = pDevice->m_Constants.VertexShaderF[StartRegister];
    // vec.x = pConstantData[0];
    // vec.y = pConstantData[1];
    // vec.z = pConstantData[2];
    // vec.w = pConstantData[3];
}

// i dunno man
__forceinline void D3DDevice_SetVertexShaderConstantF(
    D3DDevice *pDevice, UINT StartRegister, const float *pConstantData, UINT Vector4fCount
) {
    if (__IsIntConst(Vector4fCount) && Vector4fCount == 1) {
        D3DDevice_SetVertexShaderConstantF1(pDevice, StartRegister, pConstantData);
    } else {
        UINT sum = (StartRegister + Vector4fCount - 1) >> 2;
        UINT startShifted = StartRegister >> 2;
        D3DDevice_SetVertexShaderConstantFN(
            pDevice,
            StartRegister,
            pConstantData,
            Vector4fCount,
            (UINT64)(LLONG_MIN >> (sum - startShifted)) >> startShifted
        );
    }
}

static inline void D3DDevice_SetPixelShaderConstantF1(
    D3DDevice *__restrict pDevice,
    UINT StartRegister,
    const float *__restrict pConstantData
) {
    float f1 = pConstantData[0];
    float f2 = pConstantData[1];
    float f3 = pConstantData[2];
    float f4 = pConstantData[3];
    pDevice->m_Pending.m_Mask[1] |= (0x8000000000000000U >> (StartRegister >> 2));
    float *fDest = (float *)&pDevice->m_Constants.PixelShaderF[StartRegister];
    fDest[0] = f1;
    fDest[1] = f2;
    fDest[2] = f3;
    fDest[3] = f4;
    // XMVECTOR &vec = pDevice->m_Constants.PixelShaderF[StartRegister];
    // vec.x = pConstantData[0];
    // vec.y = pConstantData[1];
    // vec.z = pConstantData[2];
    // vec.w = pConstantData[3];
    // pDevice->m_Pending.m_Mask[1] |= (0x8000000000000000U >> (StartRegister >> 2));
}

__forceinline void D3DDevice_SetPixelShaderConstantF(
    D3DDevice *pDevice, UINT StartRegister, const float *pConstantData, UINT Vector4fCount
) {
    if (__IsIntConst(Vector4fCount) && Vector4fCount == 1) {
        D3DDevice_SetPixelShaderConstantF1(pDevice, StartRegister, pConstantData);
    } else {
        D3DDevice_SetPixelShaderConstantFN(
            pDevice,
            StartRegister,
            pConstantData,
            Vector4fCount,
            (0x8000000000000000U >> (StartRegister >> 2))
        );
    }
}

__forceinline void
D3DDevice_SetRenderState(D3DDevice *pDevice, D3DRENDERSTATETYPE State, DWORD Value) {
    switch (State) {
    case D3DRS_ZENABLE:
        D3DDevice_SetRenderState_ZEnable(pDevice, Value);
        break;
    case D3DRS_ZFUNC:
        D3DDevice_SetRenderState_ZFunc(pDevice, (D3DCMPFUNC)Value);
        break;
    case D3DRS_ZWRITEENABLE:
        D3DDevice_SetRenderState_ZWriteEnable(pDevice, Value);
        break;
    case D3DRS_FILLMODE:
        D3DDevice_SetRenderState_FillMode(pDevice, Value);
        break;
    case D3DRS_CULLMODE:
        D3DDevice_SetRenderState_CullMode(pDevice, Value);
        break;
    case D3DRS_ALPHABLENDENABLE:
        D3DDevice_SetRenderState_AlphaBlendEnable(pDevice, Value);
        break;
    case D3DRS_SEPARATEALPHABLENDENABLE:
        D3DDevice_SetRenderState_SeparateAlphaBlendEnable(pDevice, Value);
        break;
    case D3DRS_BLENDFACTOR:
        D3DDevice_SetRenderState_BlendFactor(pDevice, Value);
        break;
    case D3DRS_SRCBLEND:
        D3DDevice_SetRenderState_SrcBlend(pDevice, Value);
        break;
    case D3DRS_DESTBLEND:
        D3DDevice_SetRenderState_DestBlend(pDevice, Value);
        break;
    case D3DRS_BLENDOP:
        D3DDevice_SetRenderState_BlendOp(pDevice, Value);
        break;
    case D3DRS_SRCBLENDALPHA:
        D3DDevice_SetRenderState_SrcBlendAlpha(pDevice, Value);
        break;
    case D3DRS_DESTBLENDALPHA:
        D3DDevice_SetRenderState_DestBlendAlpha(pDevice, Value);
        break;
    case D3DRS_BLENDOPALPHA:
        D3DDevice_SetRenderState_BlendOpAlpha(pDevice, Value);
        break;
    case D3DRS_ALPHATESTENABLE:
        D3DDevice_SetRenderState_AlphaTestEnable(pDevice, Value);
        break;
    case D3DRS_ALPHAREF:
        D3DDevice_SetRenderState_AlphaRef(pDevice, Value);
        break;
    case D3DRS_ALPHAFUNC:
        D3DDevice_SetRenderState_AlphaFunc(pDevice, (D3DCMPFUNC)Value);
        break;
    case D3DRS_STENCILENABLE:
        D3DDevice_SetRenderState_StencilEnable(pDevice, Value);
        break;
    case D3DRS_TWOSIDEDSTENCILMODE:
        D3DDevice_SetRenderState_TwoSidedStencilMode(pDevice, Value);
        break;
    case D3DRS_STENCILFAIL:
        D3DDevice_SetRenderState_StencilFail(pDevice, Value);
        break;
    case D3DRS_STENCILZFAIL:
        D3DDevice_SetRenderState_StencilZFail(pDevice, Value);
        break;
    case D3DRS_STENCILPASS:
        D3DDevice_SetRenderState_StencilPass(pDevice, Value);
        break;
    case D3DRS_STENCILFUNC:
        D3DDevice_SetRenderState_StencilFunc(pDevice, (D3DCMPFUNC)Value);
        break;
    case D3DRS_STENCILREF:
        D3DDevice_SetRenderState_StencilRef(pDevice, Value);
        break;
    case D3DRS_STENCILMASK:
        D3DDevice_SetRenderState_StencilMask(pDevice, Value);
        break;
    case D3DRS_STENCILWRITEMASK:
        D3DDevice_SetRenderState_StencilWriteMask(pDevice, Value);
        break;
    case D3DRS_CCW_STENCILFAIL:
        D3DDevice_SetRenderState_CCWStencilFail(pDevice, Value);
        break;
    case D3DRS_CCW_STENCILZFAIL:
        D3DDevice_SetRenderState_CCWStencilZFail(pDevice, Value);
        break;
    case D3DRS_CCW_STENCILPASS:
        D3DDevice_SetRenderState_CCWStencilPass(pDevice, Value);
        break;
    case D3DRS_CCW_STENCILFUNC:
        D3DDevice_SetRenderState_CCWStencilFunc(pDevice, Value);
        break;
    case D3DRS_CCW_STENCILREF:
        D3DDevice_SetRenderState_CCWStencilRef(pDevice, Value);
        break;
    case D3DRS_CCW_STENCILMASK:
        D3DDevice_SetRenderState_CCWStencilMask(pDevice, Value);
        break;
    case D3DRS_CCW_STENCILWRITEMASK:
        D3DDevice_SetRenderState_CCWStencilWriteMask(pDevice, Value);
        break;
    case D3DRS_CLIPPLANEENABLE:
        D3DDevice_SetRenderState_ClipPlaneEnable(pDevice, Value);
        break;
    case D3DRS_POINTSIZE:
        D3DDevice_SetRenderState_PointSize(pDevice, Value);
        break;
    case D3DRS_POINTSIZE_MIN:
        D3DDevice_SetRenderState_PointSizeMin(pDevice, Value);
        break;
    case D3DRS_POINTSPRITEENABLE:
        D3DDevice_SetRenderState_PointSpriteEnable(pDevice, Value);
        break;
    case D3DRS_POINTSIZE_MAX:
        D3DDevice_SetRenderState_PointSizeMax(pDevice, Value);
        break;
    case D3DRS_MULTISAMPLEANTIALIAS:
        D3DDevice_SetRenderState_MultiSampleAntiAlias(pDevice, Value);
        break;
    case D3DRS_MULTISAMPLEMASK:
        D3DDevice_SetRenderState_MultiSampleMask(pDevice, Value);
        break;
    case D3DRS_SCISSORTESTENABLE:
        D3DDevice_SetRenderState_ScissorTestEnable(pDevice, Value);
        break;
    case D3DRS_SLOPESCALEDEPTHBIAS:
        D3DDevice_SetRenderState_SlopeScaleDepthBias(pDevice, Value);
        break;
    case D3DRS_DEPTHBIAS:
        D3DDevice_SetRenderState_DepthBias(pDevice, Value);
        break;
    case D3DRS_COLORWRITEENABLE:
        D3DDevice_SetRenderState_ColorWriteEnable(pDevice, Value);
        break;
    case D3DRS_COLORWRITEENABLE1:
        D3DDevice_SetRenderState_ColorWriteEnable1(pDevice, Value);
        break;
    case D3DRS_COLORWRITEENABLE2:
        D3DDevice_SetRenderState_ColorWriteEnable2(pDevice, Value);
        break;
    case D3DRS_COLORWRITEENABLE3:
        D3DDevice_SetRenderState_ColorWriteEnable3(pDevice, Value);
        break;
    case D3DRS_TESSELLATIONMODE:
        D3DDevice_SetRenderState_TessellationMode(pDevice, Value);
        break;
    case D3DRS_MINTESSELLATIONLEVEL:
        D3DDevice_SetRenderState_MinTessellationLevel(pDevice, Value);
        break;
    case D3DRS_MAXTESSELLATIONLEVEL:
        D3DDevice_SetRenderState_MaxTessellationLevel(pDevice, Value);
        break;
    case D3DRS_WRAP0:
        D3DDevice_SetRenderState_Wrap0(pDevice, Value);
        break;
    case D3DRS_WRAP1:
        D3DDevice_SetRenderState_Wrap1(pDevice, Value);
        break;
    case D3DRS_WRAP2:
        D3DDevice_SetRenderState_Wrap2(pDevice, Value);
        break;
    case D3DRS_WRAP3:
        D3DDevice_SetRenderState_Wrap3(pDevice, Value);
        break;
    case D3DRS_WRAP4:
        D3DDevice_SetRenderState_Wrap4(pDevice, Value);
        break;
    case D3DRS_WRAP5:
        D3DDevice_SetRenderState_Wrap5(pDevice, Value);
        break;
    case D3DRS_WRAP6:
        D3DDevice_SetRenderState_Wrap6(pDevice, Value);
        break;
    case D3DRS_WRAP7:
        D3DDevice_SetRenderState_Wrap7(pDevice, Value);
        break;
    case D3DRS_WRAP8:
        D3DDevice_SetRenderState_Wrap8(pDevice, Value);
        break;
    case D3DRS_WRAP9:
        D3DDevice_SetRenderState_Wrap9(pDevice, Value);
        break;
    case D3DRS_WRAP10:
        D3DDevice_SetRenderState_Wrap10(pDevice, Value);
        break;
    case D3DRS_WRAP11:
        D3DDevice_SetRenderState_Wrap11(pDevice, Value);
        break;
    case D3DRS_WRAP12:
        D3DDevice_SetRenderState_Wrap12(pDevice, Value);
        break;
    case D3DRS_WRAP13:
        D3DDevice_SetRenderState_Wrap13(pDevice, Value);
        break;
    case D3DRS_WRAP14:
        D3DDevice_SetRenderState_Wrap14(pDevice, Value);
        break;
    case D3DRS_WRAP15:
        D3DDevice_SetRenderState_Wrap15(pDevice, Value);
        break;
    case D3DRS_VIEWPORTENABLE:
        D3DDevice_SetRenderState_ViewportEnable(pDevice, Value);
        break;
    case D3DRS_HIGHPRECISIONBLENDENABLE:
        D3DDevice_SetRenderState_HighPrecisionBlendEnable(pDevice, Value);
        break;
    case D3DRS_HIGHPRECISIONBLENDENABLE1:
        D3DDevice_SetRenderState_HighPrecisionBlendEnable1(pDevice, Value);
        break;
    case D3DRS_HIGHPRECISIONBLENDENABLE2:
        D3DDevice_SetRenderState_HighPrecisionBlendEnable2(pDevice, Value);
        break;
    case D3DRS_HIGHPRECISIONBLENDENABLE3:
        D3DDevice_SetRenderState_HighPrecisionBlendEnable3(pDevice, Value);
        break;
    case D3DRS_HALFPIXELOFFSET:
        D3DDevice_SetRenderState_HalfPixelOffset(pDevice, Value);
        break;
    case D3DRS_PRIMITIVERESETENABLE:
        D3DDevice_SetRenderState_PrimitiveResetEnable(pDevice, Value);
        break;
    case D3DRS_PRIMITIVERESETINDEX:
        D3DDevice_SetRenderState_PrimitiveResetIndex(pDevice, Value);
        break;
    case D3DRS_ALPHATOMASKENABLE:
        D3DDevice_SetRenderState_AlphaToMaskEnable(pDevice, Value);
        break;
    case D3DRS_ALPHATOMASKOFFSETS:
        D3DDevice_SetRenderState_AlphaToMaskOffsets(pDevice, Value);
        break;
    case D3DRS_GUARDBAND_X:
        D3DDevice_SetRenderState_GuardBand_X(pDevice, Value);
        break;
    case D3DRS_GUARDBAND_Y:
        D3DDevice_SetRenderState_GuardBand_Y(pDevice, Value);
        break;
    case D3DRS_DISCARDBAND_X:
        D3DDevice_SetRenderState_DiscardBand_X(pDevice, Value);
        break;
    case D3DRS_DISCARDBAND_Y:
        D3DDevice_SetRenderState_DiscardBand_Y(pDevice, Value);
        break;
    case D3DRS_HISTENCILENABLE:
        D3DDevice_SetRenderState_HiStencilEnable(pDevice, Value);
        break;
    case D3DRS_HISTENCILWRITEENABLE:
        D3DDevice_SetRenderState_HiStencilWriteEnable(pDevice, Value);
        break;
    case D3DRS_HISTENCILFUNC:
        D3DDevice_SetRenderState_HiStencilFunc(pDevice, Value);
        break;
    case D3DRS_HISTENCILREF:
        D3DDevice_SetRenderState_HiStencilRef(pDevice, Value);
        break;
    case D3DRS_PRESENTINTERVAL:
        D3DDevice_SetRenderState_PresentInterval(pDevice, Value);
        break;
    case D3DRS_PRESENTIMMEDIATETHRESHOLD:
        D3DDevice_SetRenderState_PresentImmediateThreshold(pDevice, Value);
        break;
    case D3DRS_HIZENABLE:
        D3DDevice_SetRenderState_HiZEnable(pDevice, Value);
        break;
    case D3DRS_HIZWRITEENABLE:
        D3DDevice_SetRenderState_HiZWriteEnable(pDevice, Value);
        break;
    case D3DRS_LASTPIXEL:
        D3DDevice_SetRenderState_LastPixel(pDevice, Value);
        break;
    case D3DRS_LINEWIDTH:
        D3DDevice_SetRenderState_LineWidth(pDevice, Value);
        break;
    case D3DRS_BUFFER2FRAMES:
        D3DDevice_SetRenderState_Buffer2Frames(pDevice, Value);
        break;
    default:
        break;
    }
}

__forceinline void D3DDevice_SetSamplerState(
    D3DDevice *pDevice, DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value
) {
    switch (Type) {
    case D3DSAMP_ADDRESSU:
        D3DDevice_SetSamplerState_AddressU(pDevice, Sampler, Value);
        break;
    case D3DSAMP_ADDRESSV:
        D3DDevice_SetSamplerState_AddressV(pDevice, Sampler, Value);
        break;
    case D3DSAMP_ADDRESSW:
        D3DDevice_SetSamplerState_AddressW(pDevice, Sampler, Value);
        break;
    case D3DSAMP_BORDERCOLOR:
        D3DDevice_SetSamplerState_BorderColor(pDevice, Sampler, Value);
        break;
    case D3DSAMP_MAGFILTER:
        D3DDevice_SetSamplerState_MagFilter(pDevice, Sampler, Value);
        break;
    case D3DSAMP_MINFILTER:
        D3DDevice_SetSamplerState_MinFilter(pDevice, Sampler, Value);
        break;
    case D3DSAMP_MIPFILTER:
        D3DDevice_SetSamplerState_MipFilter(pDevice, Sampler, Value);
        break;
    case D3DSAMP_MIPMAPLODBIAS:
        D3DDevice_SetSamplerState_MipMapLodBias(pDevice, Sampler, Value);
        break;
    case D3DSAMP_MAXMIPLEVEL:
        D3DDevice_SetSamplerState_MaxMipLevel(pDevice, Sampler, Value);
        break;
    case D3DSAMP_MAXANISOTROPY:
        D3DDevice_SetSamplerState_MaxAnisotropy(pDevice, Sampler, Value);
        break;
    case D3DSAMP_MAGFILTERZ:
        D3DDevice_SetSamplerState_MagFilterZ(pDevice, Sampler, Value);
        break;
    case D3DSAMP_MINFILTERZ:
        D3DDevice_SetSamplerState_MinFilterZ(pDevice, Sampler, Value);
        break;
    case D3DSAMP_SEPARATEZFILTERENABLE:
        D3DDevice_SetSamplerState_SeparateZFilterEnable(pDevice, Sampler, Value);
        break;
    case D3DSAMP_MINMIPLEVEL:
        D3DDevice_SetSamplerState_MinMipLevel(pDevice, Sampler, Value);
        break;
    case D3DSAMP_TRILINEARTHRESHOLD:
        D3DDevice_SetSamplerState_TrilinearThreshold(pDevice, Sampler, Value);
        break;
    case D3DSAMP_ANISOTROPYBIAS:
        D3DDevice_SetSamplerState_AnisotropyBias(pDevice, Sampler, Value);
        break;
    case D3DSAMP_HGRADIENTEXPBIAS:
        D3DDevice_SetSamplerState_HGradientExpBias(pDevice, Sampler, Value);
        break;
    case D3DSAMP_VGRADIENTEXPBIAS:
        D3DDevice_SetSamplerState_VGradientExpBias(pDevice, Sampler, Value);
        break;
    case D3DSAMP_WHITEBORDERCOLORW:
        D3DDevice_SetSamplerState_WhiteBorderColorW(pDevice, Sampler, Value);
        break;
    case D3DSAMP_POINTBORDERENABLE:
        D3DDevice_SetSamplerState_PointBorderEnable(pDevice, Sampler, Value);
        break;
    default:
        break;
    }
}

__forceinline DWORD
D3DDevice_GetSamplerState(D3DDevice *pDevice, DWORD Sampler, D3DSAMPLERSTATETYPE Type) {
    switch (Type) {
    case D3DSAMP_ADDRESSU:
        return D3DDevice_GetSamplerState_AddressU(pDevice, Sampler);
    case D3DSAMP_ADDRESSV:
        return D3DDevice_GetSamplerState_AddressV(pDevice, Sampler);
    case D3DSAMP_ADDRESSW:
        return D3DDevice_GetSamplerState_AddressW(pDevice, Sampler);
    case D3DSAMP_BORDERCOLOR:
        return D3DDevice_GetSamplerState_BorderColor(pDevice, Sampler);
    case D3DSAMP_MAGFILTER:
        return D3DDevice_GetSamplerState_MagFilter(pDevice, Sampler);
    case D3DSAMP_MINFILTER:
        return D3DDevice_GetSamplerState_MinFilter(pDevice, Sampler);
    case D3DSAMP_MIPFILTER:
        return D3DDevice_GetSamplerState_MipFilter(pDevice, Sampler);
    case D3DSAMP_MIPMAPLODBIAS:
        return D3DDevice_GetSamplerState_MipMapLodBias(pDevice, Sampler);
    case D3DSAMP_MAXMIPLEVEL:
        return D3DDevice_GetSamplerState_MaxMipLevel(pDevice, Sampler);
    case D3DSAMP_MAXANISOTROPY:
        return D3DDevice_GetSamplerState_MaxAnisotropy(pDevice, Sampler);
    case D3DSAMP_MAGFILTERZ:
        return D3DDevice_GetSamplerState_MagFilterZ(pDevice, Sampler);
    case D3DSAMP_MINFILTERZ:
        return D3DDevice_GetSamplerState_MinFilterZ(pDevice, Sampler);
    case D3DSAMP_SEPARATEZFILTERENABLE:
        return D3DDevice_GetSamplerState_SeparateZFilterEnable(pDevice, Sampler);
    case D3DSAMP_MINMIPLEVEL:
        return D3DDevice_GetSamplerState_MinMipLevel(pDevice, Sampler);
    case D3DSAMP_TRILINEARTHRESHOLD:
        return D3DDevice_GetSamplerState_TrilinearThreshold(pDevice, Sampler);
    case D3DSAMP_ANISOTROPYBIAS:
        return D3DDevice_GetSamplerState_AnisotropyBias(pDevice, Sampler);
    case D3DSAMP_HGRADIENTEXPBIAS:
        return D3DDevice_GetSamplerState_HGradientExpBias(pDevice, Sampler);
    case D3DSAMP_VGRADIENTEXPBIAS:
        return D3DDevice_GetSamplerState_VGradientExpBias(pDevice, Sampler);
    case D3DSAMP_WHITEBORDERCOLORW:
        return D3DDevice_GetSamplerState_WhiteBorderColorW(pDevice, Sampler);
    case D3DSAMP_POINTBORDERENABLE:
        return D3DDevice_GetSamplerState_PointBorderEnable(pDevice, Sampler);
    default:
        return 0;
    }
}

// seeing as these regswap/instruction order swap sometimes, these need some more work

inline void
D3DDevice_SetSamplerState_BorderColor(D3DDevice *pDevice, DWORD Sampler, DWORD Value) {
    pDevice->m_Constants.TextureFetch[Sampler].BorderColor = Value ? 1 : 0;
    pDevice->m_Pending.m_Mask[3] |= 0x8000000000000000 >> (Sampler + 0x20U);
}

inline void
D3DDevice_SetSamplerState_MipFilter(D3DDevice *pDevice, DWORD Sampler, DWORD Value) {
    pDevice->m_Constants.TextureFetch[Sampler].MipFilter = Value;
    pDevice->m_Pending.m_Mask[3] |= 0x8000000000000000 >> (Sampler + 0x20U);
}

inline void
D3DDevice_SetSamplerState_AddressU(D3DDevice *pDevice, DWORD Sampler, DWORD Value) {
    pDevice->m_Constants.TextureFetch[Sampler].ClampX = Value;
    pDevice->m_Pending.m_Mask[3] |= 0x8000000000000000 >> (Sampler + 0x20U);
}

inline void
D3DDevice_SetSamplerState_AddressV(D3DDevice *pDevice, DWORD Sampler, DWORD Value) {
    pDevice->m_Constants.TextureFetch[Sampler].ClampY = Value;
    pDevice->m_Pending.m_Mask[3] |= 0x8000000000000000 >> (Sampler + 0x20U);
}

inline void
D3DDevice_SetSamplerState_AddressW(D3DDevice *pDevice, DWORD Sampler, DWORD Value) {
    pDevice->m_Constants.TextureFetch[Sampler].ClampZ = Value;
    pDevice->m_Pending.m_Mask[3] |= 0x8000000000000000 >> (Sampler + 0x20U);
}

#pragma endregion
#pragma region D3DResource

// https://learn.microsoft.com/en-us/windows/win32/api/d3d9helper/nn-d3d9helper-idirect3dresource9

// C
ULONG D3DResource_AddRef(D3DResource *pResource);
ULONG D3DResource_Release(D3DResource *pResource);
VOID D3DResource_BlockUntilNotBusy(D3DResource *pResource);
VOID D3DResource_GetDevice(D3DResource *pThis, D3DDevice **ppDevice);
D3DRESOURCETYPE D3DResource_GetType(D3DResource *pResource);
BOOL D3DResource_IsBusy(D3DResource *pResource);
BOOL D3DResource_IsSet(D3DResource *pResource, D3DDevice *pDevice);

// C++
struct D3DResource { /* Size=0x18 */
    /* 0x0000 */ UINT Common;
    /* 0x0004 */ ULONG ReferenceCount;
    /* 0x0008 */ UINT Fence;
    /* 0x000c */ UINT ReadFence;
    /* 0x0010 */ DWORD Identifier;
    /* 0x0014 */ UINT BaseFlush;

    ULONG AddRef() { return D3DResource_AddRef(this); }
    ULONG Release() { return D3DResource_Release(this); }
    HRESULT GetDevice(D3DDevice **ppDevice) {
        D3DResource_GetDevice(this, ppDevice);
        return S_OK;
    }
    D3DRESOURCETYPE GetType() { return D3DResource_GetType(this); }
    BOOL IsBusy() { return D3DResource_IsBusy(this); }
    BOOL IsSet(D3DDevice *pDevice) { return D3DResource_IsSet(this, pDevice); }
    VOID BlockUntilNotBusy() { D3DResource_BlockUntilNotBusy(this); }
    VOID SetIdentifier(DWORD Identifier); // ??? i assume just sets the Identifier field
    DWORD GetIdentifier(); // ??? ditto but gets it
};

#pragma endregion
#pragma region D3DVertexDeclaration

// https://learn.microsoft.com/en-us/windows/win32/api/d3d9helper/nn-d3d9helper-idirect3dvertexdeclaration9

VOID D3DVertexDeclaration_GetDeclaration(
    D3DVertexDeclaration *pThis, D3DVERTEXELEMENT9 *pDecl, UINT *pNumElements
);

struct D3DVertexDeclaration : public D3DResource { /* Size=0x18 */
    /* 0x0000: fields for D3DResource */
    HRESULT GetDeclaration(D3DVERTEXELEMENT9 *pDecl, UINT *pNumElements) {
        D3DVertexDeclaration_GetDeclaration(this, pDecl, pNumElements);
        return S_OK;
    }
};

#pragma endregion
#pragma region D3DVertexShader

// https://learn.microsoft.com/en-us/windows/win32/api/d3d9helper/nn-d3d9helper-idirect3dvertexshader9

void D3DVertexShader_Bind(
    D3DVertexShader *pThis,
    UINT pD3dVertexDeclaration,
    D3DVertexDeclaration *pStreamStrides,
    const UINT *pD3dPixelShader,
    D3DPixelShader *
);

struct D3DVertexShader : public D3DResource { /* Size=0x18 */
    /* 0x0000: fields for D3DResource */

    HRESULT GetFunction(void *unnamedParam1, UINT *pSizeOfData);

    HRESULT Bind(
        UINT pD3dVertexDeclaration,
        D3DVertexDeclaration *pStreamStrides,
        const UINT *pD3dPixelShader,
        D3DPixelShader *
    );
    BOOL IsBound();
};

#pragma endregion
#pragma region D3DPixelShader

// https://learn.microsoft.com/en-us/windows/win32/api/d3d9helper/nn-d3d9helper-idirect3dpixelshader9

void D3DPixelShader_GetFunction(D3DPixelShader *pThis, void *pData, UINT *pSizeOfData);

struct D3DPixelShader : public D3DResource { /* Size=0x18 */
    /* 0x0000: fields for D3DResource */
    HRESULT GetFunction(void *pData, UINT *pSizeOfData) {
        D3DPixelShader_GetFunction(this, pData, pSizeOfData);
        return S_OK;
    }
};

#pragma endregion
#pragma region D3DBaseTexture

// https://learn.microsoft.com/en-us/windows/win32/api/d3d9helper/nn-d3d9helper-idirect3dbasetexture9

VOID D3DBaseTexture_AsyncLockTail(
    D3DBaseTexture *pTexture,
    UINT64 AsyncBlock,
    UINT ArrayIndex,
    D3DLOCKED_TAIL *pLockedTail,
    DWORD Flags
);
UINT D3DBaseTexture_GetLevelCount(D3DBaseTexture *pTexture);
VOID D3DBaseTexture_GetTailDesc(D3DBaseTexture *pTexture, D3DMIPTAIL_DESC *pDesc);
VOID D3DBaseTexture_LockTail(
    D3DBaseTexture *pTexture, UINT ArrayIndex, D3DLOCKED_TAIL *pLockedTail, DWORD Flags
);
VOID D3DBaseTexture_UnlockTail(D3DBaseTexture *pTexture, UINT ArrayIndex);

struct D3DBaseTexture : public D3DResource { /* Size=0x34 */
    /* 0x0018 */ DWORD MipFlush;
    /* 0x001c */ GPUTEXTURE_FETCH_CONSTANT Format;

    UINT GetLevelCount() { return D3DBaseTexture_GetLevelCount(this); }
    HRESULT GetTailDesc(D3DMIPTAIL_DESC *pDesc) {
        D3DBaseTexture_GetTailDesc(this, pDesc);
        return S_OK;
    }
    HRESULT LockTail(UINT ArrayIndex, D3DLOCKED_TAIL *pLockedTail, DWORD Flags) {
        D3DBaseTexture_LockTail(this, ArrayIndex, pLockedTail, Flags);
        return S_OK;
    }
    HRESULT AsyncLockTail(
        UINT64 AsyncBlock, UINT ArrayIndex, D3DLOCKED_TAIL *pLockedTail, DWORD Flags
    ) {
        D3DBaseTexture_AsyncLockTail(this, AsyncBlock, ArrayIndex, pLockedTail, Flags);
        return S_OK;
    }
    HRESULT UnlockTail(UINT ArrayIndex) {
        D3DBaseTexture_UnlockTail(this, ArrayIndex);
        return S_OK;
    }
};

#pragma endregion
#pragma region D3DTexture

// https://learn.microsoft.com/en-us/windows/win32/api/d3d9helper/nn-d3d9helper-idirect3dtexture9

VOID D3DTexture_AsyncLockRect(
    D3DTexture *pTexture,
    UINT64 AsyncBlock,
    UINT Level,
    D3DLOCKED_RECT *pLockedRect,
    const RECT *pRect,
    DWORD Flags
);
VOID D3DTexture_GetLevelDesc(D3DTexture *pTexture, UINT Level, D3DSURFACE_DESC *pDesc);
D3DSurface *D3DTexture_GetSurfaceLevel(D3DTexture *pTexture, UINT Level);
VOID D3DTexture_LockRect(
    D3DTexture *pTexture,
    UINT Level,
    D3DLOCKED_RECT *pLockedRect,
    const RECT *pRect,
    DWORD Flags
);
VOID D3DTexture_UnlockRect(D3DTexture *pTexture, UINT Level);

struct D3DTexture : public D3DBaseTexture { /* Size=0x34 */
    /* 0x0000: fields for D3DBaseTexture */
    HRESULT GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) {
        D3DTexture_GetLevelDesc(this, Level, pDesc);
        return S_OK;
    }
    HRESULT GetSurfaceLevel(UINT Level, D3DSurface **ppSurfaceLevel) {
        *ppSurfaceLevel = D3DTexture_GetSurfaceLevel(this, Level);
        return *ppSurfaceLevel ? S_OK : E_OUTOFMEMORY;
    }
    HRESULT
    LockRect(UINT Level, D3DLOCKED_RECT *pLockedRect, const RECT *pRect, DWORD Flags) {
        D3DTexture_LockRect(this, Level, pLockedRect, pRect, Flags);
        return S_OK;
    }
    HRESULT AsyncLockRect(
        UINT64 AsyncBlock,
        UINT Level,
        D3DLOCKED_RECT *pLockedRect,
        const RECT *pRect,
        DWORD Flags
    ) {
        D3DTexture_AsyncLockRect(this, AsyncBlock, Level, pLockedRect, pRect, Flags);
        return S_OK;
    }
    HRESULT UnlockRect(UINT Level) {
        D3DTexture_UnlockRect(this, Level);
        return S_OK;
    }
    // need definitions
    HRESULT LockTail(D3DLOCKED_TAIL *, UINT);
    HRESULT AsyncLockTail(UINT64, D3DLOCKED_TAIL *, UINT);
    HRESULT UnlockTail();
};

#pragma endregion
#pragma region D3DVolumeTexture

// https://learn.microsoft.com/en-us/windows/win32/api/d3d9helper/nn-d3d9helper-idirect3dvolumetexture9

VOID D3DVolumeTexture_GetLevelDesc(
    D3DVolumeTexture *pTexture, UINT Level, D3DVOLUME_DESC *pDesc
);
D3DVolume *D3DVolumeTexture_GetVolumeLevel(D3DVolumeTexture *pTexture, UINT Level);
VOID D3DVolumeTexture_LockBox(
    D3DVolumeTexture *pTexture,
    UINT Level,
    D3DLOCKED_BOX *pLockedVolume,
    const D3DBOX *pBox,
    DWORD Flags
);
VOID D3DVolumeTexture_UnlockBox(D3DVolumeTexture *pTexture, UINT Level);

struct D3DVolumeTexture : public D3DBaseTexture { /* Size=0x34 */
    /* 0x0000: fields for D3DBaseTexture */
    HRESULT GetLevelDesc(UINT Level, D3DVOLUME_DESC *pDesc) {
        D3DVolumeTexture_GetLevelDesc(this, Level, pDesc);
        return S_OK;
    }
    HRESULT GetVolumeLevel(UINT Level, D3DVolume **ppVolumeLevel) {
        *ppVolumeLevel = D3DVolumeTexture_GetVolumeLevel(this, Level);
        return S_OK;
    }
    HRESULT
    LockBox(UINT Level, D3DLOCKED_BOX *pLockedVolume, const D3DBOX *pBox, DWORD Flags) {
        D3DVolumeTexture_LockBox(this, Level, pLockedVolume, pBox, Flags);
        return S_OK;
    }
    HRESULT UnlockBox(UINT Level) {
        D3DVolumeTexture_UnlockBox(this, Level);
        return S_OK;
    }
    // need definitions
    HRESULT AsyncLockBox(UINT64, UINT, D3DLOCKED_BOX *, const D3DBOX *, UINT);
    HRESULT LockTail(D3DLOCKED_TAIL *, UINT);
    HRESULT AsyncLockTail(UINT64, D3DLOCKED_TAIL *, UINT);
    HRESULT UnlockTail();
};

#pragma endregion
#pragma region D3DCubeTexture

// https://learn.microsoft.com/en-us/windows/win32/api/d3d9helper/nn-d3d9helper-idirect3dcubetexture9

VOID D3DCubeTexture_AsyncLockRect(
    D3DCubeTexture *pTexture,
    UINT64 AsyncBlock,
    D3DCUBEMAP_FACES FaceType,
    UINT Level,
    D3DLOCKED_RECT *pLockedRect,
    const RECT *pRect,
    DWORD Flags
);
D3DSurface *D3DCubeTexture_GetCubeMapSurface(
    D3DCubeTexture *pTexture, D3DCUBEMAP_FACES FaceType, UINT Level
);
VOID D3DCubeTexture_GetLevelDesc(
    D3DCubeTexture *pTexture, UINT Level, D3DSURFACE_DESC *pDesc
);
VOID D3DCubeTexture_LockRect(
    D3DCubeTexture *pTexture,
    D3DCUBEMAP_FACES FaceType,
    UINT Level,
    D3DLOCKED_RECT *pLockedRect,
    const RECT *pRect,
    DWORD Flags
);
VOID D3DCubeTexture_UnlockRect(
    D3DCubeTexture *pTexture, D3DCUBEMAP_FACES FaceType, UINT Level
);

struct D3DCubeTexture : public D3DBaseTexture { /* Size=0x34 */
    /* 0x0000: fields for D3DBaseTexture */
    HRESULT GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) {
        D3DCubeTexture_GetLevelDesc(this, Level, pDesc);
        return S_OK;
    }
    HRESULT GetCubeMapSurface(
        D3DCUBEMAP_FACES FaceType, UINT Level, D3DSurface **ppCubeMapSurface
    ) {
        *ppCubeMapSurface = D3DCubeTexture_GetCubeMapSurface(this, FaceType, Level);
        return S_OK;
    }
    HRESULT LockRect(
        D3DCUBEMAP_FACES FaceType,
        UINT Level,
        D3DLOCKED_RECT *pLockedRect,
        const RECT *pRect,
        DWORD Flags
    ) {
        D3DCubeTexture_LockRect(this, FaceType, Level, pLockedRect, pRect, Flags);
        return S_OK;
    }
    HRESULT AsyncLockRect(
        UINT64 AsyncBlock,
        D3DCUBEMAP_FACES FaceType,
        UINT Level,
        D3DLOCKED_RECT *pLockedRect,
        const RECT *pRect,
        DWORD Flags
    ) {
        D3DCubeTexture_AsyncLockRect(
            this, AsyncBlock, FaceType, Level, pLockedRect, pRect, Flags
        );
        return S_OK;
    }
    HRESULT UnlockRect(D3DCUBEMAP_FACES FaceType, UINT Level) {
        D3DCubeTexture_UnlockRect(this, FaceType, Level);
        return S_OK;
    }
    // need definitions
    HRESULT LockTail(D3DCUBEMAP_FACES, D3DLOCKED_TAIL *, UINT);
    HRESULT AsyncLockTail(UINT64, D3DCUBEMAP_FACES, D3DLOCKED_TAIL *, UINT);
    HRESULT UnlockTail(D3DCUBEMAP_FACES);
};

#pragma endregion
#pragma region D3DArrayTexture

VOID D3DArrayTexture_AsyncLockRect(
    D3DArrayTexture *pTexture,
    UINT64 AsyncBlock,
    UINT ArrayIndex,
    UINT Level,
    D3DLOCKED_RECT *pLockedRect,
    const RECT *pRect,
    DWORD Flags
);
UINT D3DArrayTexture_GetArraySize(D3DArrayTexture *pTexture);
D3DSurface *
D3DArrayTexture_GetArraySurface(D3DArrayTexture *pTexture, UINT ArrayIndex, UINT Level);
VOID D3DArrayTexture_GetLevelDesc(
    D3DArrayTexture *pTexture, UINT Level, D3DSURFACE_DESC *pDesc
);
VOID D3DArrayTexture_LockRect(
    D3DArrayTexture *pTexture,
    UINT ArrayIndex,
    UINT Level,
    D3DLOCKED_RECT *pLockedRect,
    const RECT *pRect,
    DWORD Flags
);
VOID D3DArrayTexture_UnlockRect(D3DArrayTexture *pTexture, UINT ArrayIndex, UINT Level);

struct D3DArrayTexture : public D3DBaseTexture { /* Size=0x34 */
    /* 0x0000: fields for D3DBaseTexture */
    UINT GetArraySize() { return D3DArrayTexture_GetArraySize(this); }
    HRESULT GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) {
        D3DArrayTexture_GetLevelDesc(this, Level, pDesc);
        return S_OK;
    }
    HRESULT GetArraySurface(UINT ArrayIndex, UINT Level, D3DSurface **ppArraySurface) {
        *ppArraySurface = D3DArrayTexture_GetArraySurface(this, ArrayIndex, Level);
        return S_OK;
    }
    HRESULT LockRect(
        UINT ArrayIndex,
        UINT Level,
        D3DLOCKED_RECT *pLockedRect,
        const RECT *pRect,
        DWORD Flags
    ) {
        D3DArrayTexture_LockRect(this, ArrayIndex, Level, pLockedRect, pRect, Flags);
        return S_OK;
    }
    HRESULT AsyncLockRect(
        UINT64 AsyncBlock,
        UINT ArrayIndex,
        UINT Level,
        D3DLOCKED_RECT *pLockedRect,
        const RECT *pRect,
        DWORD Flags
    ) {
        D3DArrayTexture_AsyncLockRect(
            this, AsyncBlock, ArrayIndex, Level, pLockedRect, pRect, Flags
        );
        return S_OK;
    }
    HRESULT UnlockRect(UINT ArrayIndex, UINT Level) {
        D3DArrayTexture_UnlockRect(this, ArrayIndex, Level);
        return S_OK;
    }
    // need definitions
    HRESULT GetSurfaceLevel(UINT, D3DSurface **);
    HRESULT LockTail(UINT, D3DLOCKED_TAIL *, UINT);
    HRESULT AsyncLockTail(UINT64, UINT, D3DLOCKED_TAIL *, UINT);
    HRESULT UnlockTail(UINT);
};

#pragma endregion
#pragma region D3DLineTexture

VOID D3DLineTexture_AsyncLockRect(
    D3DLineTexture *pTexture,
    UINT64 AsyncBlock,
    UINT Level,
    D3DLOCKED_RECT *pLockedRect,
    const RECT *pRect,
    DWORD Flags
);
VOID D3DLineTexture_GetLevelDesc(
    D3DLineTexture *pTexture, UINT Level, D3DSURFACE_DESC *pDesc
);
D3DSurface *D3DLineTexture_GetSurfaceLevel(D3DLineTexture *pTexture, UINT Level);
VOID D3DLineTexture_LockRect(
    D3DLineTexture *pTexture,
    UINT Level,
    D3DLOCKED_RECT *pLockedRect,
    const RECT *pRect,
    DWORD Flags
);
VOID D3DLineTexture_UnlockRect(D3DLineTexture *pTexture, UINT Level);

struct D3DLineTexture : public D3DBaseTexture { /* Size=0x34 */
    /* 0x0000: fields for D3DBaseTexture */

    HRESULT GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) {
        D3DLineTexture_GetLevelDesc(this, Level, pDesc);
        return S_OK;
    }
    HRESULT GetSurfaceLevel(UINT Level, D3DSurface **ppSurfaceLevel) {
        *ppSurfaceLevel = D3DLineTexture_GetSurfaceLevel(this, Level);
        return S_OK;
    }
    HRESULT
    LockRect(UINT Level, D3DLOCKED_RECT *pLockedRect, const RECT *pRect, DWORD Flags) {
        D3DLineTexture_LockRect(this, Level, pLockedRect, pRect, Flags);
        return S_OK;
    }
    HRESULT AsyncLockRect(
        UINT64 AsyncBlock,
        UINT Level,
        D3DLOCKED_RECT *pLockedRect,
        const RECT *pRect,
        DWORD Flags
    ) {
        D3DLineTexture_AsyncLockRect(this, AsyncBlock, Level, pLockedRect, pRect, Flags);
        return S_OK;
    }
    HRESULT UnlockRect(UINT Level) {
        D3DLineTexture_UnlockRect(this, Level);
        return S_OK;
    }
    // need definitions
    HRESULT LockTail(D3DLOCKED_TAIL *, UINT);
    HRESULT AsyncLockTail(UINT64, D3DLOCKED_TAIL *, UINT);
    HRESULT UnlockTail();
};

#pragma endregion
#pragma region D3DVertexBuffer

// https://learn.microsoft.com/en-us/windows/win32/api/d3d9helper/nn-d3d9helper-idirect3dvertexbuffer9

VOID D3DVertexBuffer_GetDesc(D3DVertexBuffer *pVertexBuffer, D3DVERTEXBUFFER_DESC *pDesc);
HANDLE D3DVertexBuffer_Lock(
    D3DVertexBuffer *pVertexBuffer, UINT OffsetToLock, UINT SizeToLock, DWORD Flags
);
VOID D3DVertexBuffer_Unlock(D3DVertexBuffer *pVertexBuffer);

struct D3DVertexBuffer : public D3DResource { /* Size=0x20 */
    /* 0x0018 */ GPUVERTEX_FETCH_CONSTANT Format;

    HRESULT Lock(UINT OffsetToLock, UINT SizeToLock, void **ppbData, DWORD Flags) {
        *ppbData = D3DVertexBuffer_Lock(this, OffsetToLock, SizeToLock, Flags);
        return S_OK;
    }
    HRESULT Unlock() {
        D3DVertexBuffer_Unlock(this);
        return S_OK;
    }
    // needs a definition
    HRESULT AsyncLock(
        UINT64 AsyncBlock, UINT OffsetToLock, UINT SizeToLock, HANDLE *ppbData, DWORD Flags
    );
    HRESULT GetDesc(D3DVERTEXBUFFER_DESC *pDesc) {
        D3DVertexBuffer_GetDesc(this, pDesc);
        return S_OK;
    }
};

#pragma endregion
#pragma region D3DIndexBuffer

// https://learn.microsoft.com/en-us/windows/win32/api/d3d9helper/nn-d3d9helper-idirect3dindexbuffer9

VOID D3DIndexBuffer_GetDesc(D3DIndexBuffer *pIndexBuffer, D3DINDEXBUFFER_DESC *pDesc);
HANDLE
D3DIndexBuffer_Lock(
    D3DIndexBuffer *pIndexBuffer, UINT OffsetToLock, UINT SizeToLock, DWORD Flags
);
HANDLE D3DIndexBuffer_AsyncLock(
    D3DIndexBuffer *pIndexBuffer,
    UINT64 AsyncBlock,
    UINT OffsetToLock,
    UINT SizeToLock,
    DWORD Flags
);
VOID D3DIndexBuffer_Unlock(D3DIndexBuffer *pIndexBuffer);

struct D3DIndexBuffer : public D3DResource { /* Size=0x20 */
    /* 0x0018 */ UINT Address;
    /* 0x001c */ UINT Size;

    HRESULT Lock(UINT OffsetToLock, UINT SizeToLock, void **ppbData, DWORD Flags) {
        *ppbData = D3DIndexBuffer_Lock(this, OffsetToLock, SizeToLock, Flags);
        return S_OK;
    }
    HRESULT Unlock() {
        D3DIndexBuffer_Unlock(this);
        return S_OK;
    }
    HRESULT AsyncLock(
        UINT64 AsyncBlock, UINT OffsetToLock, UINT SizeToLock, HANDLE *ppbData, DWORD Flags
    ) {
        *ppbData =
            D3DIndexBuffer_AsyncLock(this, AsyncBlock, OffsetToLock, SizeToLock, Flags);
        return S_OK;
    }
    HRESULT GetDesc(D3DINDEXBUFFER_DESC *pDesc) {
        D3DIndexBuffer_GetDesc(this, pDesc);
        return S_OK;
    }
};

#pragma endregion
#pragma region D3DSurface

// https://learn.microsoft.com/en-us/windows/win32/api/d3d9helper/nn-d3d9helper-idirect3dsurface9

VOID D3DSurface_AsyncLockRect(
    D3DSurface *pSurface,
    UINT64 AsyncBlock,
    D3DLOCKED_RECT *pLockedRect,
    const RECT *pRect,
    DWORD Flags
);
VOID D3DSurface_GetContainer(D3DSurface *pSurface, const _GUID &UnusedRiid);
VOID D3DSurface_GetDesc(D3DSurface *pSurface, D3DSURFACE_DESC *pDesc);
VOID D3DSurface_LockRect(
    D3DSurface *pSurface, D3DLOCKED_RECT *pLockedRect, const RECT *pRect, DWORD Flags
);
VOID D3DSurface_UnlockRect(D3DSurface *pSurface);

struct D3DSurface : public D3DResource { /* Size=0x30 */
    union {
        struct {
            /* 0x0018 */ GPU_SURFACEINFO SurfaceInfo;
            union {
                /* 0x001c */ GPU_DEPTHINFO DepthInfo;
                /* 0x001c */ GPU_COLORINFO ColorInfo;
            };
            /* 0x0020 */ GPU_HICONTROL HiControl;
            /* 0x0024 */ UINT Height : 15; /* BitPos=3 */
            /* 0x0024 */ UINT Width : 14; /* BitPos=18 */
            /* 0x0028 */ D3DFORMAT Format;
            /* 0x002c */ UINT Size;
        };
        struct {
            /* 0x0018 */ struct D3DBaseTexture *Parent;
            /* 0x001c */ UINT MipLevel : 4; /* BitPos=28 */
        };
    };
    // needs definition
    HRESULT GetContainer(const _GUID &, VOID **);
    HRESULT GetDesc(D3DSURFACE_DESC *pDesc) {
        D3DSurface_GetDesc(this, pDesc);
        return S_OK;
    }
    HRESULT LockRect(D3DLOCKED_RECT *pLockedRect, const RECT *pRect, DWORD Flags) {
        D3DSurface_LockRect(this, pLockedRect, pRect, Flags);
        return S_OK;
    }
    HRESULT AsyncLockRect(
        UINT64 AsyncBlock, D3DLOCKED_RECT *pLockedRect, const RECT *pRect, DWORD Flags
    ) {
        D3DSurface_AsyncLockRect(this, AsyncBlock, pLockedRect, pRect, Flags);
        return S_OK;
    }
    HRESULT UnlockRect() {
        D3DSurface_UnlockRect(this);
        return S_OK;
    }
};

#pragma endregion
#pragma region D3DVolume

// https://learn.microsoft.com/en-us/windows/win32/api/d3d9helper/nn-d3d9helper-idirect3dvolume9

void *D3DVolume_GetContainer(D3DVolume *pVolume, const _GUID &UnusedRiid);
void D3DVolume_GetDesc(D3DVolume *pVolume, D3DVOLUME_DESC *pDesc);
void D3DVolume_LockBox(
    D3DVolume *pVolume, D3DLOCKED_BOX *pLockedVolume, const D3DBOX *pBox, DWORD Flags
);
void D3DVolume_UnlockBox(D3DVolume *pVolume);

struct D3DVolume : public D3DResource { /* Size=0x20 */
    /* 0x0018 */ D3DBaseTexture *Parent;
    /* 0x001c */ UINT ArrayIndex : 6; /* BitPos=22 */
    /* 0x001c */ UINT MipLevel : 4; /* BitPos=28 */

    HRESULT GetDesc(D3DVOLUME_DESC *pDesc) {
        D3DVolume_GetDesc(this, pDesc);
        return S_OK;
    }
    HRESULT LockBox(D3DLOCKED_BOX *pLockedVolume, const D3DBOX *pBox, DWORD Flags) {
        D3DVolume_LockBox(this, pLockedVolume, pBox, Flags);
        return S_OK;
    }
    // need definitions
    HRESULT GetContainer(const _GUID &, VOID **);
    HRESULT AsyncLockBox(UINT64, D3DLOCKED_BOX *, const D3DBOX *, UINT);
    HRESULT UnlockBox();
};

#pragma endregion
#pragma region D3DQuery

// https://learn.microsoft.com/en-us/windows/win32/api/d3d9helper/nn-d3d9helper-idirect3dquery9

ULONG D3DQuery_AddRef(D3DQuery *pThis);
ULONG D3DQuery_Release(D3DQuery *pThis);
DWORD D3DQuery_GetDataSize(D3DQuery *pThis);
VOID D3DQuery_GetDevice(D3DQuery *pThis, D3DDevice **ppDevice);
D3DQUERYTYPE D3DQuery_GetType(D3DQuery *pThis);
HRESULT
D3DQuery_GetData(D3DQuery *pThis, VOID *pData, DWORD Size, DWORD GetDataFlags);
VOID D3DQuery_Issue(D3DQuery *pThis, DWORD IssueFlags);

struct D3DQuery { /* Size=0x1 */
    ULONG AddRef() { return D3DQuery_AddRef(this); }
    ULONG Release() { return D3DQuery_Release(this); }
    HRESULT GetDevice(D3DDevice **ppDevice) {
        D3DQuery_GetDevice(this, ppDevice);
        return S_OK;
    }
    D3DQUERYTYPE GetType() { return D3DQuery_GetType(this); }
    DWORD GetDataSize() { return D3DQuery_GetDataSize(this); }
    HRESULT Issue(DWORD IssueFlags) {
        D3DQuery_Issue(this, IssueFlags);
        return S_OK;
    }
    HRESULT GetData(VOID *pData, DWORD Size, DWORD GetDataFlags) {
        return D3DQuery_GetData(this, pData, Size, GetDataFlags);
    }
};

#pragma endregion

// https://learn.microsoft.com/en-us/windows/win32/api/d3d9helper/nn-d3d9helper-idirect3dstateblock9

struct D3DStateBlock { /* Size=0x1 */
    ULONG AddRef();
    ULONG Release();
    HRESULT GetDevice(D3DDevice **ppDevice);
    HRESULT Capture();
    HRESULT Apply();
};

#ifdef __cplusplus
}
#endif
