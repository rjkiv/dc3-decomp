#include "rndobj/Tex.h"
#include "os/System.h"
#include "os/Debug.h"
#include "rndobj/Bitmap.h"
#include "utl/BinStream.h"

void RndTex::LockBitmap(RndBitmap &bmap, int i) {
    if (mBitmap.Order() & 0x38) {
        bmap.Create(mBitmap, 0x20, 0, 0);
    } else {
        bmap.Create(
            mBitmap.Width(),
            mBitmap.Height(),
            mBitmap.RowBytes(),
            mBitmap.Bpp(),
            mBitmap.Order(),
            mBitmap.Palette(),
            mBitmap.Pixels(),
            0
        );
    }
}

TextStream &operator<<(TextStream &ts, RndTex::Type ty) {
    switch (ty) {
    case RndTex::kTexRegular:
        ts << "Regular";
        break;
    case RndTex::kTexRendered:
        ts << "Rendered";
        break;
    case RndTex::kTexMovie:
        ts << "Movie";
        break;
    case RndTex::kTexBackBuffer:
        ts << "BackBuffer";
        break;
    case RndTex::kTexFrontBuffer:
        ts << "FrontBuffer";
        break;
    case RndTex::kTexRenderedNoZ:
        ts << "RenderedNoZ";
        break;
    case RndTex::kTexShadowMap:
        ts << "ShadowMap";
        break;
    case RndTex::kTexDepthVolumeMap:
        ts << "DepthVolumeMap";
        break;
    case RndTex::kTexDensityMap:
        ts << "DensityMap";
        break;
    case RndTex::kTexScratch:
        ts << "Scratch";
        break;
    case RndTex::kTexDeviceTexture:
        ts << "DeviceTexture";
        break;
    case RndTex::kTexRegularLinear:
        ts << "RegularLinear";
        break;
    }
    return ts;
}

void RndTex::SaveBitmap(const char *bmp) {
    RndBitmap bitmap;
    LockBitmap(bitmap, 3);
    RndBitmap bitmap2;
    bitmap2.Create(bitmap, 32, 0, nullptr);
    bitmap2.SaveBmp(bmp);
    UnlockBitmap();
}

bool UseBottomMip() {
    DataArray *found = SystemConfig("rnd")->FindArray("use_bottom_mip", false);
    return found ? (found->Int(1) != 0) : false;
}

void CopyBottomMip(RndBitmap &dst, const RndBitmap &src) {
    MILO_ASSERT(&src != &dst, 0x25);
    // while (src.nextMip()) {
    //     &src = src.nextMip();
    // }
    dst.Create(src, src.Bpp(), src.Order(), nullptr);
}

RndTex::RndTex()
    : unk2c(0), mMipMapK(-8.0f), mType(kTexRegular), mWidth(0), mHeight(0), mBpp(32),
      mFilepath(), mNumMips(0), mIsPowerOf2(0), mLoader(0) {}

RndTex::~RndTex() { delete mLoader; }

void RndTex::PlatformBppOrder(const char *path, int &bpp, int &order, bool hasAlpha) {
    Platform plat = TheLoadMgr.GetPlatform();
    bool bbb;

    switch (TheLoadMgr.GetPlatform()) {
    case kPlatformWii:
        order = 8;
        if (hasAlpha) {
            order |= 0x100;
            bpp = 8;
        } else
            bpp = 4;
        order |= 0x40;
        break;

    case kPlatformPS2:
        break;

    case kPlatformXBox:
    case kPlatformPC:
    case kPlatformPS3:
        bbb = path && strstr(path, "_norm");

        if (bbb) {
            if (plat == kPlatformXBox)
                order = 0x20;
            else if (plat == kPlatformPS3)
                order = 8;
            else
                order = 0;
        } else {
            order = hasAlpha ? 0x18 : 8;
        }
        if (order == 8)
            bpp = 4;
        else if (order & 0x38U)
            bpp = 8;
        else if (bbb)
            bpp = 0x18;
        else if (bpp < 0x10)
            bpp = 0x10;
        break;

    case kPlatformNone:
        order = 0;
        break;
        // default:
        //     MILO_FAIL("bad input platform value!");
        //     break;
    }
}

const char *CheckDim(int dim, RndTex::Type ty, bool b) {
    const char *ret = 0;
    if (dim == 0)
        return ret;
    else {
        if (ty == RndTex::kTexMovie && (dim % 16 != 0)) {
            ret = "%s: dimensions not multiple of 16";
        }
        if (GetGfxMode() == 0) {
            if (b && dim > 0x400) {
                ret = "%s: dimensions greater than 1024";
            } else if (dim > 0x800) {
                ret = "%s: dimensions greater than 2048";
            }
            if (dim % 8 != 0) {
                ret = "%s: dimensions not multiple of 8";
            }
        }
        if (b) {
            if (!PowerOf2(dim))
                ret = "%s: dimensions are not power-of-2";
        }
    }
    return ret;
}

void RndTex::Save(BinStream &bs) {
    bs << 11;
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mWidth << mHeight << mBpp << mFilepath << mMipMapK << mType
       << (unsigned char)mNumMips << mIsPowerOf2;
    if (bs.Cached()) {
        mBitmap.Save(bs);
    }
}

void RndTex::Print() {
    TheDebug << "   width: " << mWidth << "\n";
    TheDebug << "   height: " << mHeight << "\n";
    TheDebug << "   bpp: " << mBpp << "\n";
    TheDebug << "   mipMapK: " << mMipMapK << "\n";
    TheDebug << "   file: " << mFilepath << "\n";
    TheDebug << "   type: " << mType << "\n";
}
