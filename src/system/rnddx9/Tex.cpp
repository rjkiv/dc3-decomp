#include "rnddx9/Tex.h"
#include "Rnd.h"
#include "Tex.h"
#include "math/Color.h"
#include "math/Geo.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Dir.h"
#include "os/Debug.h"
#include "rnddx9/Rnd.h"
#include "rndobj/Mat.h"
#include "rndobj/Mat_NG.h"
#include "rndobj/Rnd.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/ShaderOptions.h"
#include "rndobj/Tex.h"
#include "utl/MemMgr.h"
#include "xdk/D3D9.h"
#include "xdk/D3DX9.h"
#include "xdk/d3d9i/d3d9.h"
#include "xdk/d3d9i/d3d9types.h"
#include "xdk/d3dx9/d3dx9tex.h"
#include "xdk/xgraphics/xgraphics.h"

std::vector<DxTex *> gAllTextures;

DxTex::DxTex()
    : mFormat((D3DFORMAT)-1), mTexture(0), unk84(0), mRenderTarget(0), mDepthRT(0),
      unk90(0), unk9c(), unka4(0), unka8(0), unkac(0) {
    gAllTextures.push_back(this);
    for (int i = 0; i < 2; i++) {
        unk94[i] = 0;
    }
}

DxTex::~DxTex() {
    ResetSurfaces();
    auto it = std::find(gAllTextures.begin(), gAllTextures.end(), this);
    MILO_ASSERT(it != gAllTextures.end(), 0x2D7);
    gAllTextures.erase(it);
}

void DxTex::UnlockBitmap() {
    if (mTexture) {
        if (unka4) {
            unka4->UnlockRect();
            if (unka4) {
                unka4->Release();
                unka4 = nullptr;
            }
            if ((unka8 & 4) > 0) {
                HRESULT hr = D3DXFilterTexture(mTexture, nullptr, -1, -1);
                DX_ASSERT(hr, 0x618);
            }
        }
        memset(&unk9c, 0, sizeof(D3DLOCKED_RECT));
        unka4 = nullptr;
        unka8 = 0;
    }
}

void DxTex::MakeDrawTarget() {
    MILO_ASSERT(mType & kRendered, 0xF0);
    if (mTexture) {
        TheDxRnd.Resume();
        TheDxRnd.Device()->SetPredication(3);
        TheDxRnd.Device()->SetRenderTarget(0, mRenderTarget);
        if (mType != kDepthVolumeMap) {
            TheDxRnd.Device()->SetDepthStencilSurface(mDepthRT);
        } else {
            TheDxRnd.Device()->SetDepthStencilSurface(nullptr);
        }
        TheDxRnd.SetUnk301(mType != kShadowMap);
        NgMat::SetCurrent(nullptr);
    }
}

void DxTex::FinishDrawTarget() {
    MILO_ASSERT(mType & kRendered, 0x122);
    ResolveMipChain();
    TheDxRnd.Device()->SetPredication(0);
    TheDxRnd.SetUnk301(true);
}

void DxTex::Compress(AlphaCompress a) {
    void *v = StartCompress(a);
    DoCompress(v);
    FinishCompress(v);
}

bool DxTex::TexelsLock(void *&v) {
    if (mTexture) {
        UINT baseData;
        XGGetTextureLayout(
            mTexture,
            &baseData,
            nullptr,
            nullptr,
            nullptr,
            0,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            0
        );
        v = (void *)baseData;
        return true;
    } else {
        v = nullptr;
        return false;
    }
}

unsigned int DxTex::TexelsPitch() const {
    D3DLOCKED_RECT rect;
    mTexture->LockRect(0, &rect, nullptr, 0);
    mTexture->UnlockRect(0);
    return rect.Pitch;
}

void DxTex::Select(int stage) {
    D3DTexture *tex = mTexture;
    if (!tex) {
        tex = static_cast<DxTex *>(TheRnd.GetNullTexture())->mTexture;
    }
    if (mType & kBackBuffer) {
        if (mType == kFrontBuffer) {
            tex = TheDxRnd.FrontBuffer();
        } else {
            TheDxRnd.Device()->Resolve(
                0, nullptr, mTexture, nullptr, 0, 0, nullptr, 1, 0, nullptr
            );
        }
    }
    TheDxRnd.Device()->SetTexture(stage, tex);
}

void DxTex::PreDeviceReset() {
    if (IsBackBuffer() || IsRenderTarget()) {
        ResetSurfaces();
    }
}

void DxTex::PostDeviceReset() {
    if (IsBackBuffer()) {
        SetBitmap(TheRnd.Width(), TheRnd.Height(), TheRnd.Bpp(), mType, false, nullptr);
    }
    if (IsRenderTarget()) {
        SyncBitmap();
    }
}

void DxTex::SetDeviceTex(D3DTexture *tex) {
    mTexture = tex;
    mType = kDeviceTexture;
    if (tex) {
        D3DSURFACE_DESC desc;
        tex->GetLevelDesc(0, &desc);
        mNumMips = 0;
        mFormat = desc.Format;
        mWidth = desc.Width;
        mHeight = desc.Height;
        mBpp = D3DFORMAT_BitsPerPixel(desc.Format);
    }
}

D3DSurface *DxTex::GetRT() {
    if (!IsRenderTarget()) {
        return nullptr;
    } else {
        mRenderTarget->AddRef();
        return mRenderTarget;
    }
}

D3DSurface *DxTex::GetDepthRT() { return mDepthRT; }

D3DSurface *DxTex::GetSurfaceLevel(int level) {
    D3DSurface *ret;
    HRESULT hr = mTexture->GetSurfaceLevel(level, &ret);
    DX_ASSERT(hr, 0xE6);
    return ret;
}

D3DSurface *DxTex::GetMovieSurface() {
    if (!(mType & kMovie)) {
        return nullptr;
    } else {
        mTexture = unk94[unk90];
        return GetSurfaceLevel(0);
    }
}

void DxTex::SwapMovieSurface() {
    MILO_ASSERT((mType & kMovie) > 0, 0x2F5);
    unk90 = (unk90 + 1) % 2;
    mTexture = unk94[unk90];
}

void DxTex::ResetSurfaces() {
    for (int i = 0; i < 2; i++) {
        if (mTexture == unk94[i]) {
            mTexture = nullptr;
        }
        DX_RELEASE(unk94[i]);
    }
    DX_DELETE(mTexture);
}

void *DxTex::StartCompress(AlphaCompress ac) {
    MILO_ASSERT(mTexture, 0x15A);
    for (int i = 0; i < 16; i++) {
        TheShaderMgr.SetPConstant((PShaderConstant)i, (RndTex *)nullptr);
    }
    {
        MemDoTempAllocations tmp;
    }
    UINT numLevels = mTexture->GetLevelCount();
    return 0;
}

void DxTex::ResolveMipChain() {
    if (mType != kShadowMap) {
        TheDxRnd.Device()->Resolve(
            0, nullptr, mTexture, nullptr, 0, 0, nullptr, 1, 0, nullptr
        );
        TheDxRnd.Device()->SetRenderTarget(0, mRenderTarget);
        TheDxRnd.Device()->SetDepthStencilSurface(nullptr);
    } else {
        TheDxRnd.Device()->Resolve(
            4, nullptr, mTexture, nullptr, 0, 0, nullptr, 1, 0, nullptr
        );
        MILO_ASSERT(!mRenderTarget, 0x237);
        TheDxRnd.Device()->SetRenderTarget(0, nullptr);
        TheDxRnd.Device()->SetDepthStencilSurface(mDepthRT);
        TheDxRnd.Device()->SetSamplerState(0, D3DSAMP_MINFILTER, 1);
        TheDxRnd.Device()->SetSamplerState(0, D3DSAMP_MAGFILTER, 1);
        TheDxRnd.Device()->SetSamplerState(0, D3DSAMP_MIPFILTER, 1);
    }
    if (mNumMips != 0) {
        UINT levelCount = mTexture->GetLevelCount();
        for (UINT i = 1; i < levelCount; i++) {
            TheDxRnd.Device()->SetSamplerState(0, D3DSAMP_MINMIPLEVEL, i - 1);
            TheDxRnd.Device()->SetSamplerState(0, D3DSAMP_MAXMIPLEVEL, i - 1);
            D3DSURFACE_DESC desc;
            mTexture->GetLevelDesc(i, &desc);
            D3DRECT dRect;
            dRect.x1 = dRect.y1 = 0;
            dRect.x2 = desc.Width;
            dRect.y2 = desc.Height;
            RndMat *workMat = TheShaderMgr.GetWork();
            workMat->SetDiffuseTex(this);
            workMat->SetTexWrap(kTexWrapClamp);
            workMat->SetBlend(RndMat::kBlendSrc);
            if (mType == kShadowMap) {
                workMat->SetZMode(kZModeForce);
            } else {
                workMat->SetZMode(kZModeDisable);
            }
            Hmx::Rect rect(0, 0, desc.Width, desc.Height);
            if (mType != kShadowMap) {
                Hmx::Color color;
                TheDxRnd.DrawRect(
                    rect, workMat, kDownsampleShader, color, nullptr, nullptr
                );
                TheDxRnd.Device()->Resolve(
                    0, &dRect, mTexture, nullptr, i, 0, nullptr, 1, 0, nullptr
                );
            } else {
                Hmx::Color color;
                TheDxRnd.DrawRect(
                    rect, workMat, kDownsampleDepthShader, color, nullptr, nullptr
                );
                TheDxRnd.Device()->Resolve(
                    4, &dRect, mTexture, nullptr, i, 0, nullptr, 1, 0, nullptr
                );
            }
        }
        TheDxRnd.Device()->SetSamplerState(0, D3DSAMP_MINMIPLEVEL, 13);
        TheDxRnd.Device()->SetSamplerState(0, D3DSAMP_MAXMIPLEVEL, 0);
        TheDxRnd.Device()->SetSamplerState(0, D3DSAMP_MINFILTER, 1);
        TheDxRnd.Device()->SetSamplerState(0, D3DSAMP_MAGFILTER, 1);
        TheDxRnd.Device()->SetSamplerState(0, D3DSAMP_MIPFILTER, 1);
    }
}

DataNode DebugPrintAllTextures(DataArray *) {
    static int gTexDumpCalls = 0;
    MILO_LOG("tex_dump_%d: %s\n", gTexDumpCalls);
    MILO_LOG("ObjName, dir, mTexture, size, width, height, bpp, mips, type, file\n");
    int numTextures = 0;
    FOREACH (it, gAllTextures) {
        DxTex *cur = *it;
        const char *name = cur->Name();
        if (name && *name) {
            const char *dirName = cur->Dir() ? cur->Dir()->Name() : "";
            MILO_LOG(
                "%s, %s, %x, %d, %d, %d, %d, %d, %d, %s\n",
                name,
                dirName,
                cur->Tex(),
                cur->SizeKb() * 1024, // size in bytes
                cur->Width(),
                cur->Height(),
                cur->Bpp(),
                cur->NumMips(),
                cur->GetType(),
                cur->File().c_str()
            );
            numTextures++;
        }
    }
    gTexDumpCalls++;
    return numTextures;
}

void DxTex::Init() { DataRegisterFunc("dump_tex", DebugPrintAllTextures); }
