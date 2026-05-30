#include "rnddx9/CubeTex.h"
#include "os/Memory.h"
#include "Rnd.h"
#include "rnddx9/Rnd.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Mat_NG.h"
#include "xdk/D3D9.h"
#include "xdk/XGRAPHICS.h"

DxCubeTex::DxCubeTex() : mTex(0) {}
DxCubeTex::~DxCubeTex() { Reset(); }

void DxCubeTex::Select(int x) {
    D3DDevice_SetTexture(TheDxRnd.Device(), x, mTex, x + 0x20U);
}

void DxCubeTex::Reset() {
    TheDxRnd.AutoRelease(mTex);
    mTex = nullptr;
    NgMat::SetCurrent(nullptr);
}

void DxCubeTex::Sync() {
    PhysMemTypeTracker tracker("D3D(phys):CubeTex");
    mTex = D3DDevice_CreateTexture(
        props.mWidth,
        props.mWidth,
        6,
        props.mNumMips + 1,
        0,
        TheDxRnd.D3DFormatForBitmap(mBitmap[kCubeFaceRight]),
        0,
        D3DRTYPE_CUBETEXTURE
    );
    DX_ASSERT(mTex, 0x38);
    XGTEXTURE_DESC desc;
    XGGetTextureDesc(mTex, 0, &desc);
    for (int i = 0; i < kNumCubeFaces; i++) {
        RndBitmap bitmap;
    }
    NgMat::SetCurrent(nullptr);
}
