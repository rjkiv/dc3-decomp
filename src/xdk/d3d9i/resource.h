#pragma once

#include "xdk/d3d9i/device.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "xdk/d3d9i/d3dtypes.h"

typedef struct D3DResource D3DResource;
typedef struct D3DIndexBuffer D3DIndexBuffer;
typedef struct D3DVertexBuffer D3DVertexBuffer;

/// https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dindexbuffer-desc
typedef struct D3DINDEXBUFFER_DESC {
    D3DFORMAT Format;
    D3DRESOURCETYPE Type;
    DWORD Usage;
    D3DPOOL Pool;
    UINT Size;
} D3DINDEXBUFFER_DESC, *LPD3DINDEXBUFFER_DESC;

/// https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dvertexbuffer-desc
typedef struct D3DVERTEXBUFFER_DESC {
    D3DFORMAT Format;
    D3DRESOURCETYPE Type;
    DWORD Usage;
    D3DPOOL Pool;
    UINT Size;
    DWORD FVF;
} D3DVERTEXBUFFER_DESC, *LPD3DVERTEXBUFFER_DESC;

BOOL D3DResource_IsSet(D3DResource *, D3DDevice *);

// both guesses cause there's no mangling and the args are different from ID3DDevice9
struct D3DVertexBuffer *
D3DDevice_CreateVertexBuffer(UINT Length, DWORD Usage, D3DPOOL Pool);

struct D3DIndexBuffer *
D3DDevice_CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool);

HRESULT D3DIndexBuffer_GetDesc(D3DIndexBuffer *, D3DINDEXBUFFER_DESC *pDesc);
HRESULT
D3DIndexBuffer_Lock(D3DIndexBuffer *, UINT OffsetToLock, UINT SizeToLock, DWORD Flags);
HRESULT D3DIndexBuffer_Unlock(D3DIndexBuffer *);

HRESULT D3DVertexBuffer_GetDesc(D3DVertexBuffer *, D3DVERTEXBUFFER_DESC *pDesc);
HRESULT
D3DVertexBuffer_Lock(D3DVertexBuffer *, UINT OffsetToLock, UINT SizeToLock, DWORD Flags);
HRESULT D3DVertexBuffer_Unlock(D3DVertexBuffer *);

HRESULT D3DDevice_SetStreamSource(
    D3DDevice *,
    UINT StreamNumber,
    struct D3DVertexBuffer *pStreamData,
    UINT OffsetInBytes,
    UINT Stride,
    uint unk_r8
);

#ifdef __cplusplus
}
#endif
