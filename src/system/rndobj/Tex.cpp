#include "rndobj/Tex.h"
#include "Tex.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/File.h"
#include "os/System.h"
#include "os/Debug.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Rnd.h"
#include "rndobj/Utl.h"
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

#pragma region RndTex

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
        } else if (mType != c->mType) {
            return;
        }
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

BEGIN_LOADS(RndTex)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

void RndTex::Print() {
    TheDebug << "   width: " << mWidth << "\n";
    TheDebug << "   height: " << mHeight << "\n";
    TheDebug << "   bpp: " << mBpp << "\n";
    TheDebug << "   mipMapK: " << mMipMapK << "\n";
    TheDebug << "   file: " << mFilepath << "\n";
    TheDebug << "   type: " << mType << "\n";
}

INIT_REVS(11, 0)

void RndTex::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(11, 0)
    if (d.rev > 8) {
        LOAD_SUPERCLASS(Hmx::Object)
    }
    if (d.rev == 1) {
        short w, h;
        d >> w;
        d >> h;
        mWidth = w;
        mHeight = h;
    } else {
        d >> mWidth;
        d >> mHeight;
    }
    d >> mBpp;
    d >> mFilepath;
    if (d.rev > 9) {
        if (!bs.Cached()) {
            mLoader = new FileLoader(
                mFilepath,
                CacheResource(mFilepath.c_str(), this),
                kLoadFront,
                0,
                false,
                true,
                nullptr,
                nullptr
            );
        }
    }
    PUSH_REVS(d);
}

void RndTex::PostLoad(BinStream &bs) {
    POP_REVS(bs);
    if (d.rev < 5) {
        int cubemapmask;
        bs >> cubemapmask;
        if (cubemapmask != 0 && !mFilepath.empty()) {
            if (cubemapmask & 1) {
                MILO_NOTIFY("%s: kTransparentWhite no longer supported", Name());
            } else if (cubemapmask & 2) {
                mFilepath.insert(mFilepath.find('.'), "_tb");
            } else if (cubemapmask & 0x10) {
                mFilepath.insert(mFilepath.find('.'), "_ga");
            } else if (cubemapmask & 0x20) {
                mFilepath.insert(mFilepath.find('.'), "_gw");
            } else if (cubemapmask & 0x40) {
                MILO_NOTIFY("%s: kCubeMap no longer supported", Name());
            }
        }
    }
    if (d.rev > 0 && d.rev < 3) {
        bool b;
        d >> b;
    }
    if (d.rev > 7) {
        d >> mMipMapK;
    } else if (d.rev > 3) {
        int i;
        d >> i;
        mMipMapK = i / 16.0f;
    }
    if (d.rev > 6) {
        d >> (int &)mType;
    } else if (d.rev > 5) {
        Type types[5] = { kRegular, kRendered, kMovie, kBackBuffer, kFrontBuffer };
        int i;
        d >> i;
        mType = types[i];
    } else if (d.rev > 4) {
        bool b;
        d >> b;
        mType = b ? kRendered : kRegular;
    }
    bool b7 = false;
    if (d.rev > 7) {
        d >> b7;
    }
    if (d.rev > 10) {
        d >> mOptimizeForPS3;
    }
    if (bs.Cached()) {
        PresyncBitmap();
        if (UseBottomMip()) {
            RndBitmap bmap;
            d >> bmap;
            CopyBottomMip(mBitmap, bmap);
        } else {
            d >> mBitmap;
        }
        if (!mBitmap.HasName() && mType == kRegular) {
            MILO_LOG(
                "Bitmap %s, does not have name set, it will not be cached!\n", Name()
            );
        }
        mNumMips = mBitmap.NumMips();
        SyncBitmap();
    } else if (mFilepath.empty() || mType != kRegular) {
        SetBitmap(mWidth, mHeight, mBpp, mType, b7, nullptr);
    } else if (TheLoadMgr.GetPlatform() != kPlatformNone) {
        SetBitmap(mLoader);
        mLoader = nullptr;
    } else {
        RELEASE(mLoader);
    }
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
    bool bbb;
    switch (TheLoadMgr.GetPlatform()) {
    case kPlatformNone: // 0x0
        order = 0;
        break;
    case kPlatformXBox: // 0x2
    case kPlatformPC: // 0x3
    case kPlatformPS3: // 0x4
        bbb = path && strstr(path, "_norm");
        if (bbb) {
            Platform plat = TheLoadMgr.GetPlatform();
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
    case kPlatformWii: // 0x5
        order = 8;
        if (hasAlpha) {
            order |= 0x140;
            bpp = 8;
        } else {
            bpp = 4;
        }
        order |= 0x40;
        break;
    case kPlatform3DS: // 0x6
        order = 0x600;
        bpp = hasAlpha ? 8 : 4;
        break;
    default: // 0x1 - no ps2 here lol
        MILO_FAIL("bad input platform value!");
        break;
    }
}

bool RndTex::PowerOf2() { return ::PowerOf2(mWidth) && ::PowerOf2(mHeight); }

const char *CheckDim(int dim, RndTex::Type ty, bool b) {
    const char *err = nullptr;
    if (dim == 0) {
        return nullptr;
    } else {
        if (b) {
            if (ty == RndTex::kMovie) {
                if (dim % 16 != 0) {
                    err = "%s: dimensions not multiple of 16";
                }
            } else if (dim % 8 != 0) {
                err = "%s: dimensions not multiple of 8";
            }
        }
        if (GetGfxMode() == kOldGfx) {
            if (b && dim > 0x400) {
                err = "%s: dimensions greater than 1024";
            } else if (dim > 0x1000) {
                err = "%s: dimensions greater than 4096";
            }
        }
        if (b && !::PowerOf2(dim)) {
            err = "%s: dimensions are not power-of-2";
        }
        return err;
    }
}

const char *
RndTex::CheckSize(int width, int height, int bpp, int numMips, Type ty, bool file) {
    if (ty == kDepthVolumeMap || ty == kDensityMap || (ty & 0x1000) || (ty & 0x2000)) {
        return 0;
    } else {
        const char *err = CheckDim(width, ty, file);
        if (!err) {
            err = CheckDim(height, ty, file);
            if (!err && bpp != 4 && bpp != 8 && bpp != 16 && bpp != 24 && bpp != 32) {
                err = "%s: invalid bpp";
            }
        }
        int sizeBytes = (width * height * bpp) >> 3;
        if (GetGfxMode() == kOldGfx && !err) {
            if (sizeBytes > 0x7FFF0) {
                err = "%s: size over 524,272 bytes";
            } else if (sizeBytes & 0xF) {
                err = "%s: size not multiple of 16 bytes";
            } else if (numMips > 6) {
                err = "%s: more than 6 mip levels";
            }
        }
        return err;
    }
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

void RndTex::SetBitmap(int w, int h, int bpp, Type ty, bool useMips, const char *path) {
    PresyncBitmap();
    mWidth = w;
    mHeight = h;
    mBpp = bpp;
    mType = ty;
    mFilepath.Set(FilePath::Root().c_str(), "");
    mNumMips = 0;
    mBitmap.Reset();
    if (mType & 8) {
        mWidth = TheRnd.Width();
        mHeight = TheRnd.Height();
        mBpp = TheRnd.Bpp();
    } else if (mType & 2) {
        if (mBpp & 0xF) {
            mBpp = mBpp < 0x20 ? 0x10 : 0x20;
        }
        if (useMips) {
            for (int i = mWidth, j = mHeight; i > 0x10 && j > 0x10; i >>= 1, j >>= 1) {
                mNumMips++;
            }
        }
    } else {
        const char *err = CheckSize(mWidth, mHeight, mBpp, mNumMips, mType, false);
        if (err) {
            MILO_NOTIFY(err, Name());
        } else if (!(mType & 0x204)) {
            int platformBpp = mBpp;
            int platformOrder;
            PlatformBppOrder(path, platformBpp, platformOrder, true);
            mBitmap.Create(
                mWidth, mHeight, 0, platformBpp, platformOrder, nullptr, nullptr, nullptr
            );
            if (useMips) {
                mBitmap.GenerateMips();
                mNumMips = mBitmap.NumMips();
            }
        }
    }
    SyncBitmap();
}

void RndTex::SetBitmap(const RndBitmap &bmap, const char *path, bool b, Type ty) {
    PresyncBitmap();
    mWidth = bmap.Width();
    mHeight = bmap.Height();
    mBpp = bmap.Bpp();
    mType = ty;
    mFilepath.Set(FilePath::Root().c_str(), "");
    mNumMips = bmap.NumMips();
    const char *err = CheckSize(mWidth, mHeight, mBpp, mNumMips, mType, false);
    if (err) {
        MILO_NOTIFY(err, Name());
        mBitmap.Reset();
    } else {
        int platformBpp = bmap.Bpp();
        int platformOrder = bmap.Order();
        if (!b) {
            PlatformBppOrder(path, platformBpp, platformOrder, bmap.IsTranslucent());
        }
        mBitmap.Create(bmap, platformBpp, platformOrder, nullptr);
    }
    SyncBitmap();
}

DataNode RndTex::OnSetRendered(const DataArray *a) {
    MILO_ASSERT(IsRenderTarget(), 0x3BB);
    SetBitmap(mWidth, mHeight, mBpp, mType, mNumMips > 0, nullptr);
    return 0;
}

DataNode RndTex::OnSetBitmap(const DataArray *da) {
    if (da->Size() == 3) {
        FilePath p(da->Str(2));
        SetBitmap(p);
    } else {
        SetBitmap(
            da->Int(2),
            da->Int(3),
            da->Int(4),
            (RndTex::Type)da->Int(5),
            da->Int(6),
            nullptr
        );
    }
    return 0;
}

#pragma endregion
