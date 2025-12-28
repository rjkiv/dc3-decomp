#pragma once
#include "d3d9types.h"
#include "d3d9caps.h"

// Larger struct definitions and functions go here.

#ifdef __cplusplus
extern "C" {
#endif

// source for most if not all of this: the kinect joyride pdb

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

#pragma region D3DDevice

typedef struct D3DDevice { /* Size=0x2a80 */
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
} D3DDevice;

#pragma endregion
#pragma region D3DResource

// D3DResource methods
ULONG D3DResource_AddRef(struct D3DResource *pResource);
ULONG D3DResource_Release(struct D3DResource *pResource);
VOID D3DResource_BlockUntilNotBusy(struct D3DResource *pResource);
VOID D3DResource_GetDevice(struct D3DResource *pThis, D3DDevice **ppDevice);
D3DRESOURCETYPE D3DResource_GetType(struct D3DResource *pResource);
BOOL D3DResource_IsBusy(struct D3DResource *pResource);
BOOL D3DResource_IsSet(struct D3DResource *pResource, D3DDevice *pDevice);

typedef struct D3DResource { /* Size=0x18 */
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
        return 0;
    }
    D3DRESOURCETYPE GetType() { return D3DResource_GetType(this); }
    BOOL IsBusy() { return D3DResource_IsBusy(this); }
    BOOL IsSet(D3DDevice *pDevice) { return D3DResource_IsSet(this, pDevice); }
    VOID BlockUntilNotBusy() { D3DResource_BlockUntilNotBusy(this); }
    VOID SetIdentifier(DWORD Identifier); // ??? i assume just sets the Identifier field
    DWORD GetIdentifier(); // ??? ditto but gets it
} D3DResource;

#pragma endregion
#pragma region D3DIndexBuffer

VOID D3DIndexBuffer_GetDesc(
    struct D3DIndexBuffer *pIndexBuffer, D3DINDEXBUFFER_DESC *pDesc
);
HANDLE
D3DIndexBuffer_Lock(
    struct D3DIndexBuffer *pIndexBuffer, UINT OffsetToLock, UINT SizeToLock, DWORD Flags
);
HANDLE D3DIndexBuffer_AsyncLock(
    struct D3DIndexBuffer *pIndexBuffer,
    UINT64 AsyncBlock,
    UINT OffsetToLock,
    UINT SizeToLock,
    DWORD Flags
);
VOID D3DIndexBuffer_Unlock(struct D3DIndexBuffer *pIndexBuffer);

typedef struct D3DIndexBuffer : public D3DResource { /* Size=0x20 */
    /* 0x0018 */ UINT Address;
    /* 0x001c */ UINT Size;

    HRESULT Lock(UINT OffsetToLock, UINT SizeToLock, void **ppbData, DWORD Flags) {
        *ppbData = D3DIndexBuffer_Lock(this, OffsetToLock, SizeToLock, Flags);
        return 0;
    }
    HRESULT Unlock() {
        D3DIndexBuffer_Unlock(this);
        return 0;
    }
    HRESULT AsyncLock(
        UINT64 AsyncBlock, UINT OffsetToLock, UINT SizeToLock, HANDLE *ppbData, DWORD Flags
    ) {
        *ppbData =
            D3DIndexBuffer_AsyncLock(this, AsyncBlock, OffsetToLock, SizeToLock, Flags);
        return 0;
    }
    HRESULT GetDesc(D3DINDEXBUFFER_DESC *pDesc) {
        D3DIndexBuffer_GetDesc(this, pDesc);
        return 0;
    }
} D3DIndexBuffer;

#pragma endregion
#pragma region D3DVertexBuffer

VOID D3DVertexBuffer_GetDesc(
    struct D3DVertexBuffer *pVertexBuffer, D3DVERTEXBUFFER_DESC *pDesc
);
HANDLE D3DVertexBuffer_Lock(
    struct D3DVertexBuffer *pVertexBuffer, UINT OffsetToLock, UINT SizeToLock, DWORD Flags
);
VOID D3DVertexBuffer_Unlock(struct D3DVertexBuffer *pVertexBuffer);

typedef struct D3DVertexBuffer : public D3DResource { /* Size=0x20 */
    /* 0x0018 */ GPUVERTEX_FETCH_CONSTANT Format;

    HRESULT Lock(UINT OffsetToLock, UINT SizeToLock, void **ppbData, DWORD Flags) {
        *ppbData = D3DVertexBuffer_Lock(this, OffsetToLock, SizeToLock, Flags);
        return 0;
    }
    HRESULT Unlock() {
        D3DVertexBuffer_Unlock(this);
        return 0;
    }
    // needs a definition
    HRESULT AsyncLock(
        UINT64 AsyncBlock, UINT OffsetToLock, UINT SizeToLock, HANDLE *ppbData, DWORD Flags
    );
    HRESULT GetDesc(D3DVERTEXBUFFER_DESC *pDesc) {
        D3DVertexBuffer_GetDesc(this, pDesc);
        return 0;
    }
} D3DVertexBuffer;

#pragma endregion
#pragma region D3DSurface

VOID D3DSurface_AsyncLockRect(
    struct D3DSurface *pSurface,
    UINT64 AsyncBlock,
    D3DLOCKED_RECT *pLockedRect,
    const tagRECT *pRect,
    DWORD Flags
);
VOID D3DSurface_GetContainer(struct D3DSurface *pSurface, const _GUID &UnusedRiid);
VOID D3DSurface_GetDesc(struct D3DSurface *pSurface, D3DSURFACE_DESC *pDesc);
VOID D3DSurface_LockRect(
    struct D3DSurface *pSurface,
    D3DLOCKED_RECT *pLockedRect,
    const tagRECT *pRect,
    DWORD Flags
);
VOID D3DSurface_UnlockRect(struct D3DSurface *pSurface);

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
        return 0;
    }
    HRESULT LockRect(D3DLOCKED_RECT *pLockedRect, const tagRECT *pRect, DWORD Flags) {
        D3DSurface_LockRect(this, pLockedRect, pRect, Flags);
        return 0;
    }
    HRESULT AsyncLockRect(
        UINT64 AsyncBlock, D3DLOCKED_RECT *pLockedRect, const tagRECT *pRect, DWORD Flags
    ) {
        D3DSurface_AsyncLockRect(this, AsyncBlock, pLockedRect, pRect, Flags);
        return 0;
    }
    HRESULT UnlockRect() {
        D3DSurface_UnlockRect(this);
        return 0;
    }
};

#pragma endregion
#pragma region D3DTexture

VOID D3DBaseTexture_AsyncLockTail(
    struct D3DBaseTexture *pTexture,
    UINT64 AsyncBlock,
    UINT ArrayIndex,
    D3DLOCKED_TAIL *pLockedTail,
    DWORD Flags
);
UINT D3DBaseTexture_GetLevelCount(struct D3DBaseTexture *pTexture);
VOID D3DBaseTexture_GetTailDesc(struct D3DBaseTexture *pTexture, D3DMIPTAIL_DESC *pDesc);
VOID D3DBaseTexture_LockTail(
    struct D3DBaseTexture *pTexture,
    UINT ArrayIndex,
    D3DLOCKED_TAIL *pLockedTail,
    DWORD Flags
);
VOID D3DBaseTexture_UnlockTail(struct D3DBaseTexture *pTexture, UINT ArrayIndex);

struct D3DBaseTexture : public D3DResource { /* Size=0x34 */
    /* 0x0018 */ DWORD MipFlush;
    /* 0x001c */ GPUTEXTURE_FETCH_CONSTANT Format;

    UINT GetLevelCount() { return D3DBaseTexture_GetLevelCount(this); }
    HRESULT GetTailDesc(D3DMIPTAIL_DESC *pDesc) {
        D3DBaseTexture_GetTailDesc(this, pDesc);
        return 0;
    }
    HRESULT LockTail(UINT ArrayIndex, D3DLOCKED_TAIL *pLockedTail, DWORD Flags) {
        D3DBaseTexture_LockTail(this, ArrayIndex, pLockedTail, Flags);
        return 0;
    }
    HRESULT AsyncLockTail(
        UINT64 AsyncBlock, UINT ArrayIndex, D3DLOCKED_TAIL *pLockedTail, DWORD Flags
    ) {
        D3DBaseTexture_AsyncLockTail(this, AsyncBlock, ArrayIndex, pLockedTail, Flags);
        return 0;
    }
    HRESULT UnlockTail(UINT ArrayIndex) {
        D3DBaseTexture_UnlockTail(this, ArrayIndex);
        return 0;
    }
};

VOID D3DTexture_AsyncLockRect(
    struct D3DTexture *pTexture,
    UINT64 AsyncBlock,
    UINT Level,
    D3DLOCKED_RECT *pLockedRect,
    const tagRECT *pRect,
    DWORD Flags
);
VOID D3DTexture_GetLevelDesc(
    struct D3DTexture *pTexture, UINT Level, D3DSURFACE_DESC *pDesc
);
D3DSurface *D3DTexture_GetSurfaceLevel(struct D3DTexture *pTexture, UINT Level);
VOID D3DTexture_LockRect(
    struct D3DTexture *pTexture,
    UINT Level,
    D3DLOCKED_RECT *pLockedRect,
    const tagRECT *pRect,
    DWORD Flags
);
VOID D3DTexture_UnlockRect(struct D3DTexture *pTexture, UINT Level);

struct D3DTexture : public D3DBaseTexture { /* Size=0x34 */
    /* 0x0000: fields for D3DBaseTexture */
    HRESULT GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) {
        D3DTexture_GetLevelDesc(this, Level, pDesc);
        return 0;
    }
    HRESULT GetSurfaceLevel(UINT Level, D3DSurface **ppSurfaceLevel) {
        *ppSurfaceLevel = D3DTexture_GetSurfaceLevel(this, Level);
        return 0;
    }
    HRESULT
    LockRect(UINT Level, D3DLOCKED_RECT *pLockedRect, const tagRECT *pRect, DWORD Flags) {
        D3DTexture_LockRect(this, Level, pLockedRect, pRect, Flags);
        return 0;
    }
    HRESULT AsyncLockRect(
        UINT64 AsyncBlock,
        UINT Level,
        D3DLOCKED_RECT *pLockedRect,
        const tagRECT *pRect,
        DWORD Flags
    ) {
        D3DTexture_AsyncLockRect(this, AsyncBlock, Level, pLockedRect, pRect, Flags);
        return 0;
    }
    HRESULT UnlockRect(UINT Level) {
        D3DTexture_UnlockRect(this, Level);
        return 0;
    }
    // need definitions
    HRESULT LockTail(D3DLOCKED_TAIL *, UINT);
    HRESULT AsyncLockTail(UINT64, D3DLOCKED_TAIL *, UINT);
    HRESULT UnlockTail();
};

VOID D3DArrayTexture_AsyncLockRect(
    struct D3DArrayTexture *pTexture,
    UINT64 AsyncBlock,
    UINT ArrayIndex,
    UINT Level,
    D3DLOCKED_RECT *pLockedRect,
    const tagRECT *pRect,
    DWORD Flags
);
UINT D3DArrayTexture_GetArraySize(struct D3DArrayTexture *pTexture);
D3DSurface *D3DArrayTexture_GetArraySurface(
    struct D3DArrayTexture *pTexture, UINT ArrayIndex, UINT Level
);
VOID D3DArrayTexture_GetLevelDesc(
    struct D3DArrayTexture *pTexture, UINT Level, D3DSURFACE_DESC *pDesc
);
VOID D3DArrayTexture_LockRect(
    struct D3DArrayTexture *pTexture,
    UINT ArrayIndex,
    UINT Level,
    D3DLOCKED_RECT *pLockedRect,
    const tagRECT *pRect,
    DWORD Flags
);
VOID D3DArrayTexture_UnlockRect(
    struct D3DArrayTexture *pTexture, UINT ArrayIndex, UINT Level
);

struct D3DArrayTexture : public D3DBaseTexture { /* Size=0x34 */
    /* 0x0000: fields for D3DBaseTexture */
    UINT GetArraySize() { return D3DArrayTexture_GetArraySize(this); }
    HRESULT GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) {
        D3DArrayTexture_GetLevelDesc(this, Level, pDesc);
        return 0;
    }
    HRESULT GetArraySurface(UINT ArrayIndex, UINT Level, D3DSurface **ppArraySurface) {
        *ppArraySurface = D3DArrayTexture_GetArraySurface(this, ArrayIndex, Level);
        return 0;
    }
    HRESULT LockRect(
        UINT ArrayIndex,
        UINT Level,
        D3DLOCKED_RECT *pLockedRect,
        const tagRECT *pRect,
        DWORD Flags
    ) {
        D3DArrayTexture_LockRect(this, ArrayIndex, Level, pLockedRect, pRect, Flags);
        return 0;
    }
    HRESULT AsyncLockRect(
        UINT64 AsyncBlock,
        UINT ArrayIndex,
        UINT Level,
        D3DLOCKED_RECT *pLockedRect,
        const tagRECT *pRect,
        DWORD Flags
    ) {
        D3DArrayTexture_AsyncLockRect(
            this, AsyncBlock, ArrayIndex, Level, pLockedRect, pRect, Flags
        );
        return 0;
    }
    HRESULT UnlockRect(UINT ArrayIndex, UINT Level) {
        D3DArrayTexture_UnlockRect(this, ArrayIndex, Level);
        return 0;
    }
    // need definitions
    HRESULT GetSurfaceLevel(UINT, D3DSurface **);
    HRESULT LockTail(UINT, D3DLOCKED_TAIL *, UINT);
    HRESULT AsyncLockTail(UINT64, UINT, D3DLOCKED_TAIL *, UINT);
    HRESULT UnlockTail(UINT);
};

VOID D3DCubeTexture_AsyncLockRect(
    struct D3DCubeTexture *pTexture,
    UINT64 AsyncBlock,
    D3DCUBEMAP_FACES FaceType,
    UINT Level,
    D3DLOCKED_RECT *pLockedRect,
    const tagRECT *pRect,
    DWORD Flags
);
D3DSurface *D3DCubeTexture_GetCubeMapSurface(
    struct D3DCubeTexture *pTexture, D3DCUBEMAP_FACES FaceType, UINT Level
);
VOID D3DCubeTexture_GetLevelDesc(
    struct D3DCubeTexture *pTexture, UINT Level, D3DSURFACE_DESC *pDesc
);
VOID D3DCubeTexture_LockRect(
    struct D3DCubeTexture *pTexture,
    D3DCUBEMAP_FACES FaceType,
    UINT Level,
    D3DLOCKED_RECT *pLockedRect,
    const tagRECT *pRect,
    DWORD Flags
);
VOID D3DCubeTexture_UnlockRect(
    struct D3DCubeTexture *pTexture, D3DCUBEMAP_FACES FaceType, UINT Level
);

struct D3DCubeTexture : public D3DBaseTexture { /* Size=0x34 */
    /* 0x0000: fields for D3DBaseTexture */
    HRESULT GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) {
        D3DCubeTexture_GetLevelDesc(this, Level, pDesc);
        return 0;
    }
    HRESULT GetCubeMapSurface(
        D3DCUBEMAP_FACES FaceType, UINT Level, D3DSurface **ppCubeMapSurface
    ) {
        *ppCubeMapSurface = D3DCubeTexture_GetCubeMapSurface(this, FaceType, Level);
        return 0;
    }
    HRESULT LockRect(
        D3DCUBEMAP_FACES FaceType,
        UINT Level,
        D3DLOCKED_RECT *pLockedRect,
        const tagRECT *pRect,
        DWORD Flags
    ) {
        D3DCubeTexture_LockRect(this, FaceType, Level, pLockedRect, pRect, Flags);
        return 0;
    }
    HRESULT AsyncLockRect(
        UINT64 AsyncBlock,
        D3DCUBEMAP_FACES FaceType,
        UINT Level,
        D3DLOCKED_RECT *pLockedRect,
        const tagRECT *pRect,
        DWORD Flags
    ) {
        D3DCubeTexture_AsyncLockRect(
            this, AsyncBlock, FaceType, Level, pLockedRect, pRect, Flags
        );
        return 0;
    }
    HRESULT UnlockRect(D3DCUBEMAP_FACES FaceType, UINT Level) {
        D3DCubeTexture_UnlockRect(this, FaceType, Level);
        return 0;
    }
    // need definitions
    HRESULT LockTail(D3DCUBEMAP_FACES, D3DLOCKED_TAIL *, UINT);
    HRESULT AsyncLockTail(UINT64, D3DCUBEMAP_FACES, D3DLOCKED_TAIL *, UINT);
    HRESULT UnlockTail(D3DCUBEMAP_FACES);
};

VOID D3DLineTexture_AsyncLockRect(
    struct D3DLineTexture *pTexture,
    UINT64 AsyncBlock,
    UINT Level,
    D3DLOCKED_RECT *pLockedRect,
    const tagRECT *pRect,
    DWORD Flags
);
VOID D3DLineTexture_GetLevelDesc(
    struct D3DLineTexture *pTexture, UINT Level, D3DSURFACE_DESC *pDesc
);
D3DSurface *D3DLineTexture_GetSurfaceLevel(struct D3DLineTexture *pTexture, UINT Level);
VOID D3DLineTexture_LockRect(
    struct D3DLineTexture *pTexture,
    UINT Level,
    D3DLOCKED_RECT *pLockedRect,
    const tagRECT *pRect,
    DWORD Flags
);
VOID D3DLineTexture_UnlockRect(struct D3DLineTexture *pTexture, UINT Level);

struct D3DLineTexture : public D3DBaseTexture { /* Size=0x34 */
    /* 0x0000: fields for D3DBaseTexture */

    HRESULT GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) {
        D3DLineTexture_GetLevelDesc(this, Level, pDesc);
        return 0;
    }
    HRESULT GetSurfaceLevel(UINT Level, D3DSurface **ppSurfaceLevel) {
        *ppSurfaceLevel = D3DLineTexture_GetSurfaceLevel(this, Level);
        return 0;
    }
    HRESULT
    LockRect(UINT Level, D3DLOCKED_RECT *pLockedRect, const tagRECT *pRect, DWORD Flags) {
        D3DLineTexture_LockRect(this, Level, pLockedRect, pRect, Flags);
        return 0;
    }
    HRESULT AsyncLockRect(
        UINT64 AsyncBlock,
        UINT Level,
        D3DLOCKED_RECT *pLockedRect,
        const tagRECT *pRect,
        DWORD Flags
    ) {
        D3DLineTexture_AsyncLockRect(this, AsyncBlock, Level, pLockedRect, pRect, Flags);
        return 0;
    }
    HRESULT UnlockRect(UINT Level) {
        D3DLineTexture_UnlockRect(this, Level);
        return 0;
    }
    // need definitions
    HRESULT LockTail(D3DLOCKED_TAIL *, UINT);
    HRESULT AsyncLockTail(UINT64, D3DLOCKED_TAIL *, UINT);
    HRESULT UnlockTail();
};

VOID D3DVolumeTexture_GetLevelDesc(
    struct D3DVolumeTexture *pTexture, UINT Level, D3DVOLUME_DESC *pDesc
);
struct D3DVolume *
D3DVolumeTexture_GetVolumeLevel(struct D3DVolumeTexture *pTexture, UINT Level);
VOID D3DVolumeTexture_LockBox(
    struct D3DVolumeTexture *pTexture,
    UINT Level,
    D3DLOCKED_BOX *pLockedVolume,
    const D3DBOX *pBox,
    DWORD Flags
);
VOID D3DVolumeTexture_UnlockBox(struct D3DVolumeTexture *pTexture, UINT Level);

struct D3DVolumeTexture : public D3DBaseTexture { /* Size=0x34 */
    /* 0x0000: fields for D3DBaseTexture */
    HRESULT GetLevelDesc(UINT Level, D3DVOLUME_DESC *pDesc) {
        D3DVolumeTexture_GetLevelDesc(this, Level, pDesc);
        return 0;
    }
    HRESULT GetVolumeLevel(UINT Level, struct D3DVolume **ppVolumeLevel) {
        *ppVolumeLevel = D3DVolumeTexture_GetVolumeLevel(this, Level);
        return 0;
    }
    HRESULT
    LockBox(UINT Level, D3DLOCKED_BOX *pLockedVolume, const D3DBOX *pBox, DWORD Flags) {
        D3DVolumeTexture_LockBox(this, Level, pLockedVolume, pBox, Flags);
        return 0;
    }
    HRESULT UnlockBox(UINT Level) {
        D3DVolumeTexture_UnlockBox(this, Level);
        return 0;
    }
    // need definitions
    HRESULT AsyncLockBox(UINT64, UINT, D3DLOCKED_BOX *, const D3DBOX *, UINT);
    HRESULT LockTail(D3DLOCKED_TAIL *, UINT);
    HRESULT AsyncLockTail(UINT64, D3DLOCKED_TAIL *, UINT);
    HRESULT UnlockTail();
};

#pragma endregion
#pragma region D3DVolume

void *D3DVolume_GetContainer(struct D3DVolume *pVolume, const _GUID &UnusedRiid);
void D3DVolume_GetDesc(struct D3DVolume *pVolume, D3DVOLUME_DESC *pDesc);
void D3DVolume_LockBox(
    struct D3DVolume *pVolume,
    D3DLOCKED_BOX *pLockedVolume,
    const D3DBOX *pBox,
    DWORD Flags
);
void D3DVolume_UnlockBox(struct D3DVolume *pVolume);

struct D3DVolume : public D3DResource { /* Size=0x20 */
    /* 0x0018 */ D3DBaseTexture *Parent;
    /* 0x001c */ UINT ArrayIndex : 6; /* BitPos=22 */
    /* 0x001c */ UINT MipLevel : 4; /* BitPos=28 */

    HRESULT GetDesc(D3DVOLUME_DESC *pDesc) {
        D3DVolume_GetDesc(this, pDesc);
        return 0;
    }
    HRESULT LockBox(D3DLOCKED_BOX *pLockedVolume, const D3DBOX *pBox, DWORD Flags) {
        D3DVolume_LockBox(this, pLockedVolume, pBox, Flags);
        return 0;
    }
    // need definitions
    HRESULT GetContainer(const _GUID &, VOID **);
    HRESULT AsyncLockBox(UINT64, D3DLOCKED_BOX *, const D3DBOX *, UINT);
    HRESULT UnlockBox();
};

#pragma endregion
#pragma region D3DQuery

ULONG D3DQuery_AddRef(struct D3DQuery *pThis);
ULONG D3DQuery_Release(struct D3DQuery *pThis);
DWORD D3DQuery_GetDataSize(struct D3DQuery *pThis);
VOID D3DQuery_GetDevice(struct D3DQuery *pThis, D3DDevice **ppDevice);
D3DQUERYTYPE D3DQuery_GetType(struct D3DQuery *pThis);
HRESULT
D3DQuery_GetData(struct D3DQuery *pThis, VOID *pData, DWORD Size, DWORD GetDataFlags);
VOID D3DQuery_Issue(struct D3DQuery *pThis, DWORD IssueFlags);

struct D3DQuery { /* Size=0x1 */
    ULONG AddRef() { return D3DQuery_AddRef(this); }
    ULONG Release() { return D3DQuery_Release(this); }
    HRESULT GetDevice(D3DDevice **ppDevice) {
        D3DQuery_GetDevice(this, ppDevice);
        return 0;
    }
    D3DQUERYTYPE GetType() { return D3DQuery_GetType(this); }
    DWORD GetDataSize() { return D3DQuery_GetDataSize(this); }
    HRESULT Issue(DWORD IssueFlags) {
        D3DQuery_Issue(this, IssueFlags);
        return 0;
    }
    HRESULT GetData(VOID *pData, DWORD Size, DWORD GetDataFlags) {
        return D3DQuery_GetData(this, pData, Size, GetDataFlags);
    }
};

#pragma endregion
#pragma region D3DPerfCounters

ULONG D3DPerfCounters_AddRef(struct D3DPerfCounters *pThis);
ULONG D3DPerfCounters_Release(struct D3DPerfCounters *pThis);
VOID D3DPerfCounters_GetDevice(struct D3DPerfCounters *pThis, D3DDevice **ppDevice);
VOID D3DPerfCounters_BlockUntilNotBusy(struct D3DPerfCounters *pThis);
UINT D3DPerfCounters_GetNumPasses(struct D3DPerfCounters *pThis);
BOOL D3DPerfCounters_IsBusy(struct D3DPerfCounters *pThis);
HRESULT
D3DPerfCounters_GetValues(
    struct D3DPerfCounters *pThis,
    D3DPERFCOUNTER_VALUES *pValues,
    UINT PassIndex,
    UINT *pPassType
);

struct D3DPerfCounters { /* Size=0x1 */
    ULONG AddRef() { return D3DPerfCounters_AddRef(this); }
    ULONG Release() { return D3DPerfCounters_Release(this); }
    HRESULT GetDevice(D3DDevice **ppDevice) {
        D3DPerfCounters_GetDevice(this, ppDevice);
        return 0;
    }
    BOOL IsBusy() { return D3DPerfCounters_IsBusy(this); }
    VOID BlockUntilNotBusy() { D3DPerfCounters_BlockUntilNotBusy(this); }
    UINT GetNumPasses() { return D3DPerfCounters_GetNumPasses(this); }
    INT GetValues(D3DPERFCOUNTER_VALUES *pValues, UINT PassIndex, UINT *pPassType) {
        return D3DPerfCounters_GetValues(this, pValues, PassIndex, pPassType);
    }
};

VOID D3DVertexDeclaration_GetDeclaration(
    struct D3DVertexDeclaration *pThis, D3DVERTEXELEMENT9 *pDecl, UINT *pNumElements
);

struct D3DVertexDeclaration : public D3DResource { /* Size=0x18 */
    /* 0x0000: fields for D3DResource */
    HRESULT GetDeclaration(D3DVERTEXELEMENT9 *pDecl, UINT *pNumElements) {
        D3DVertexDeclaration_GetDeclaration(this, pDecl, pNumElements);
        return 0;
    }
};

#pragma endregion
#pragma region PerfCounters

typedef struct _D3DPERFCOUNTER_EVENTS { /* Size=0xf0 */
    /* 0x0000 */ GPUPERFEVENT_CP CP[1];
    /* 0x0004 */ GPUPERFEVENT_RBBM RBBM[2];
    /* 0x000c */ GPUPERFEVENT_SQ SQ[4];
    /* 0x001c */ GPUPERFEVENT_VGT VGT[4];
    /* 0x002c */ GPUPERFEVENT_VC VC[4];
    /* 0x003c */ GPUPERFEVENT_PA_SU PA_SU[4];
    /* 0x004c */ GPUPERFEVENT_PA_SC PA_SC[4];
    /* 0x005c */ GPUPERFEVENT_HZ HZ[2];
    /* 0x0064 */ GPUPERFEVENT_TCR TCR[2];
    /* 0x006c */ GPUPERFEVENT_TCM TCM[2];
    /* 0x0074 */ GPUPERFEVENT_TCF TCF[12];
    /* 0x00a4 */ GPUPERFEVENT_TP TP0[2];
    /* 0x00ac */ GPUPERFEVENT_TP TP1[2];
    /* 0x00b4 */ GPUPERFEVENT_TP TP2[2];
    /* 0x00bc */ GPUPERFEVENT_TP TP3[2];
    /* 0x00c4 */ GPUPERFEVENT_SX SX[1];
    /* 0x00c8 */ GPUPERFEVENT_BC BC[4];
    /* 0x00d8 */ GPUPERFEVENT_MC MC0[1];
    /* 0x00dc */ GPUPERFEVENT_MC MC1[1];
    /* 0x00e0 */ GPUPERFEVENT_MH MH[3];
    /* 0x00ec */ GPUPERFEVENT_BIF BIF[1];
} D3DPERFCOUNTER_EVENTS;

void D3DDevice_EnablePerfCounters(D3DDevice *pDevice, BOOL Enable);
void D3DDevice_SetPerfCounterEvents(
    D3DDevice *pDevice, const D3DPERFCOUNTER_EVENTS *pEvents, DWORD Flags
);
void D3DDevice_QueryPerfCounters(
    D3DDevice *pDevice, D3DPerfCounters *pCounters, DWORD Flags
);

#pragma endregion
#pragma region D3DPixelShader

void D3DPixelShader_GetFunction(
    struct D3DPixelShader *pThis, void *pData, UINT *pSizeOfData
);

struct D3DPixelShader : public D3DResource { /* Size=0x18 */
    /* 0x0000: fields for D3DResource */
    HRESULT GetFunction(void *pData, UINT *pSizeOfData) {
        D3DPixelShader_GetFunction(this, pData, pSizeOfData);
        return 0;
    }
};

D3DPixelShader *D3DDevice_CreatePixelShader(const DWORD *pFunction);

#pragma endregion
#pragma region D3DVertexShader

struct D3DVertexShader : public D3DResource { /* Size=0x18 */
    /* 0x0000: fields for D3DResource */

    //   int32_t GetFunction(void*, uint32_t*);
    //   int32_t Bind(uint32_t, D3DVertexDeclaration*, const uint32_t*, D3DPixelShader*);
    //   int32_t IsBound();
};

D3DVertexShader *D3DDevice_CreateVertexShader(const DWORD *pFunction);

#pragma endregion
#pragma region RenderState

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
void D3DDevice_SetRenderState_ZEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_ZFunc(D3DDevice *pDevice, D3DCMPFUNC Value);
void D3DDevice_SetRenderState_ZWriteEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_PointSizeMax(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_SeparateAlphaBlendEnable(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_BlendOpAlpha(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_PresentInterval(D3DDevice *pDevice, DWORD Value);
void D3DDevice_SetRenderState_PresentImmediateThreshold(D3DDevice *pDevice, DWORD Value);

#pragma endregion
#pragma region Misc

void D3DDevice_SetSamplerState_MinFilter(D3DDevice *pDevice, DWORD Sampler, DWORD Value);
void D3DDevice_SetSamplerState_MagFilter(D3DDevice *pDevice, DWORD Sampler, DWORD Value);

D3DVertexBuffer *D3DDevice_CreateVertexBuffer(UINT Length, DWORD Usage, D3DPOOL Pool);

D3DIndexBuffer *
D3DDevice_CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool);

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
    UINT Count,
    const D3DRECT *pRects,
    UINT Flags,
    UINT Color,
    float Z,
    UINT Stencil,
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

D3DQuery *
D3DDevice_CreateQueryTiled(D3DDevice *pDevice, D3DQUERYTYPE Type, UINT TileCapacity);

void D3DDevice_SetVertexShader(D3DDevice *pDevice, D3DVertexShader *pShader);
void D3DDevice_SetPixelShader(D3DDevice *pDevice, D3DPixelShader *pShader);

#ifdef __cplusplus
}
#endif
