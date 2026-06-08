#include "rnddx9/CubeTex.h"
#include "os/Debug.h"
#include "os/Memory.h"
#include "Rnd.h"
#include "rnddx9/Rnd.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Mat_NG.h"
#include "xdk/D3D9.h"
#include "xdk/XGRAPHICS.h"
#include "xdk/d3d9i/d3d9types.h"
#include "xdk/xgraphics/xgraphics.h"

DxCubeTex::DxCubeTex() : mTex(0) {}
DxCubeTex::~DxCubeTex() { Reset(); }

void DxCubeTex::Select(int stage) { TheDxRnd.Device()->SetTexture(stage, mTex); }

void DxCubeTex::Reset() {
    DX_RELEASE(mTex);
    NgMat::SetCurrent(nullptr);
}

void DxCubeTex::Sync() {
    PhysMemTypeTracker tracker("D3D(phys):CubeTex");
    D3DFORMAT fmt = TheDxRnd.D3DFormatForBitmap(mBitmap[kCubeFaceRight]);
    int numLevels = props.mNumMips + 1;
    HRESULT hr = TheDxRnd.Device()->CreateCubeTexture(
        props.mWidth, numLevels, 0, fmt, 0, &mTex, nullptr
    );
    DX_ASSERT(hr, 0x38);
    XGTEXTURE_DESC desc;
    XGGetTextureDesc(mTex, 0, &desc);
    for (int i = 0; i < kNumCubeFaces; i++) {
        RndBitmap bitmap;
        RndBitmap &curFace = mBitmap[i];
        if (curFace.Width() && curFace.Height()) {
            RndBitmap *bmp;
            if (!curFace.Palette()) {
                bmp = &curFace;
                if (curFace.Bpp() == 0x18) {
                    goto ok;
                }
            } else {
            ok:
                bitmap.Create(curFace, 0x20, curFace.Order(), nullptr);
                bmp = &bitmap;
            }
            for (int j = 0; j < numLevels; j++) {
                MILO_ASSERT(bmp, 0x53);
                D3DLOCKED_RECT d3dRect;
                mTex->LockRect((D3DCUBEMAP_FACES)i, j, &d3dRect, nullptr, 0);
                unsigned char gpuFmt = desc.Format;
                XGTileTextureLevel(
                    desc.Width,
                    desc.Height,
                    j,
                    gpuFmt,
                    0,
                    d3dRect.pBits,
                    nullptr,
                    bmp->Pixels(),
                    bmp->DxtRowBytes(),
                    nullptr
                );
                mTex->UnlockRect((D3DCUBEMAP_FACES)i, j);
                bmp = bmp->nextMip();
            }
            curFace.Reset();
        } else {
            MILO_NOTIFY("%s face %d width or height == 0 ", PathName(this), i);
        }
    }
    NgMat::SetCurrent(nullptr);
}
