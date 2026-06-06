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

void DxCubeTex::Select(int stage) { TheDxRnd.Device()->SetTexture(stage, mTex); }

void DxCubeTex::Reset() {
    DX_RELEASE(mTex);
    NgMat::SetCurrent(nullptr);
}

void DxCubeTex::Sync() {
    PhysMemTypeTracker tracker("D3D(phys):CubeTex");
    HRESULT hr = TheDxRnd.Device()->CreateCubeTexture(
        props.mWidth,
        props.mNumMips + 1,
        0,
        TheDxRnd.D3DFormatForBitmap(mBitmap[kCubeFaceRight]),
        0,
        &mTex,
        nullptr
    );
    DX_ASSERT(hr, 0x38);
    XGTEXTURE_DESC desc;
    XGGetTextureDesc(mTex, 0, &desc);
    for (int i = 0; i < kNumCubeFaces; i++) {
        RndBitmap bitmap;
    }
    NgMat::SetCurrent(nullptr);
}
