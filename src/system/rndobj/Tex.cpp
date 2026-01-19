#include "rndobj/Tex.h"
#include "Tex.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/File.h"
#include "os/System.h"
#include "os/Debug.h"
#include "rndobj/Bitmap.h"
#include "utl/BinStream.h"
#include "utl/CRC.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"

bool UseBottomMip() {
    DataArray *found = SystemConfig("rnd")->FindArray("use_bottom_mip", false);
    if (found)
        return found->Int(1);
    else
        return false;
}

void CopyBottomMip(RndBitmap &dst, const RndBitmap &src) {
    MILO_ASSERT(&src != &dst, 0x25);
    const RndBitmap *srcPtr = &src;
    while (srcPtr->nextMip())
        srcPtr = srcPtr->nextMip();
    dst.Create(*srcPtr, srcPtr->Bpp(), srcPtr->Order(), nullptr);
}

RndTex::RndTex()
    : mMipMapK(-8.0f), mType(kRegular), mWidth(0), mHeight(0), mBpp(32), mFilepath(),
      mNumMips(0), mOptimizeForPS3(0), mLoader(0) {}

RndTex::~RndTex() { delete mLoader; }

BEGIN_HANDLERS(RndTex)
    HANDLE(set_bitmap, OnSetBitmap)
    HANDLE(set_rendered, OnSetRendered)
    HANDLE_EXPR(file_path, mFilepath.c_str())
    HANDLE_ACTION(set_file_path, mFilepath.Set(FilePath::Root().c_str(), _msg->Str(2)))
    HANDLE_EXPR(size_kb, SizeKb())
    HANDLE_EXPR(tex_type, mType)
    HANDLE_ACTION(save_bmp, SaveBitmap(_msg->Str(2)))
    HANDLE_ACTION(save_png, _msg->Str(2)) // musta got stubbed out
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndTex)
    SYNC_PROP_SET(width, mWidth, OnSetSize(_val.Int(), mHeight))
    SYNC_PROP_SET(height, mHeight, OnSetSize(mWidth, _val.Int())) {
        static Symbol _s("bpp");
        if (sym == _s && _op & kPropGet)
            return PropSync(mBpp, _val, _prop, _i + 1, _op);
    }
    SYNC_PROP(mip_map_k, mMipMapK)
    SYNC_PROP(optimize_for_ps3, mOptimizeForPS3)
    SYNC_PROP_MODIFY(file_path, mFilepath, SetBitmap(mFilepath))
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

DataNode RndTex::OnSetSize(int, int) { return 0; }

BEGIN_SAVES(RndTex)
    SAVE_REVS(11, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mWidth << mHeight << mBpp << mFilepath << mMipMapK << mType;
    bs << (bool)mNumMips;
    bs << mOptimizeForPS3;
    if (bs.Cached()) {
        mBitmap.Save(bs);
    }
END_SAVES

BEGIN_COPYS(RndTex)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(RndTex)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax) {
            COPY_MEMBER(mMipMapK)
        } else if (mType != c->mType)
            return;
        PresyncBitmap();
        COPY_MEMBER(unk2c)
        COPY_MEMBER(mType)
        COPY_MEMBER(mWidth)
        COPY_MEMBER(mHeight)
        COPY_MEMBER(mBpp)
        COPY_MEMBER(mFilepath)
        COPY_MEMBER(mNumMips)
        COPY_MEMBER(mOptimizeForPS3)
        mBitmap.Create(c->mBitmap, c->mBitmap.Bpp(), c->mBitmap.Order(), nullptr);
        SyncBitmap();
    END_COPYING_MEMBERS
END_COPYS

void RndTex::Print() {
    TheDebug << "   width: " << mWidth << "\n";
    TheDebug << "   height: " << mHeight << "\n";
    TheDebug << "   bpp: " << mBpp << "\n";
    TheDebug << "   mipMapK: " << mMipMapK << "\n";
    TheDebug << "   file: " << mFilepath << "\n";
    TheDebug << "   type: " << mType << "\n";
}

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
    case RndTex::kRegular:
        ts << "Regular";
        break;
    case RndTex::kRendered:
        ts << "Rendered";
        break;
    case RndTex::kMovie:
        ts << "Movie";
        break;
    case RndTex::kBackBuffer:
        ts << "BackBuffer";
        break;
    case RndTex::kFrontBuffer:
        ts << "FrontBuffer";
        break;
    case RndTex::kRenderedNoZ:
        ts << "RenderedNoZ";
        break;
    case RndTex::kShadowMap:
        ts << "ShadowMap";
        break;
    case RndTex::kDepthVolumeMap:
        ts << "DepthVolumeMap";
        break;
    case RndTex::kDensityMap:
        ts << "DensityMap";
        break;
    case RndTex::kScratch:
        ts << "Scratch";
        break;
    case RndTex::kDeviceTexture:
        ts << "DeviceTexture";
        break;
    case RndTex::kRegularLinear:
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
        if (ty == RndTex::kMovie && (dim % 16 != 0)) {
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

void RndTex::SetBitmap(FileLoader *fl) {
    PresyncBitmap();
    mType = kRegular;
    char *buffer;
    if (fl) {
        mFilepath = fl->LoaderFile();
        TheLoadMgr.PollUntilLoaded(fl, nullptr);
        buffer = fl->GetBuffer(nullptr);
        if (!TheLoadMgr.EditMode() && fl != mLoader) {
            if (!strstr(mFilepath.c_str(), "_keep")) {
                MILO_NOTIFY("%s will not be included on a disc build", mFilepath);
            }
        }
        delete fl;
    } else {
        mFilepath.Set(FilePath::Root().c_str(), "");
        buffer = nullptr;
    }

    if (buffer) {
        if (UseBottomMip()) {
            RndBitmap bmap;
            bmap.Create(buffer);
            CopyBottomMip(mBitmap, bmap);
        } else {
            mBitmap.Create(buffer);
        }
        if (!mBitmap.HasName()) {
            if (!mFilepath.empty()) {
                mBitmap.SetName(
                    Hmx::CRC(FileRelativePath(FileExecRoot(), mFilepath.c_str()))
                );
            }
        }
        if (!mBitmap.HasName() && mType == kRegular) {
            MILO_LOG(
                "Bitmap %s, does not have name set, it will not be cached!\n",
                FileRelativePath(FileRoot(), mFilepath.c_str())
            );
        }
        mWidth = mBitmap.Width();
        mHeight = mBitmap.Height();
        mBpp = mBitmap.Bpp();
        mNumMips = mBitmap.NumMips();
    } else {
        mBitmap.Reset();
        mWidth = mHeight = 0;
        mBpp = 32;
        mNumMips = 0;
    }
    SyncBitmap();
}

void RndTex::SetBitmap(const FilePath &path) {
    Loader *ldr = TheLoadMgr.ForceGetLoader(path);
    SetBitmap(dynamic_cast<FileLoader *>(ldr));
}

DataNode RndTex::OnSetRendered(const DataArray *a) {
    MILO_ASSERT(IsRenderTarget(), 0x3BB);
    SetBitmap(mWidth, mHeight, mBpp, mType, mNumMips > 0, nullptr);
    return 0;
}
