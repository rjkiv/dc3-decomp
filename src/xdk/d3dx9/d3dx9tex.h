#pragma once
#include "xdk/D3D9.h"

#ifdef __cplusplus
extern "C" {
#endif

HRESULT D3DXFilterTexture(
    D3DBaseTexture *pBaseTexture,
    const PALETTEENTRY *pPalette,
    UINT uSrcLevel,
    DWORD dwFilter
);

#ifdef __cplusplus
}
#endif
