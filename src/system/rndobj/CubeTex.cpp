#include "rndobj/CubeTex.h"
#include "CubeTex.h"
#include "obj/Object.h"
#include "os/File.h"
#include "rndobj/Tex.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"

#pragma region CubeTexProperties

void RndCubeTex::CubeTexProperties::Set(const RndBitmap &bmap) {
    mBpp = bmap.Bpp();
    mHeight = bmap.Height();
    mWidth = bmap.Width();
    mNumMips = bmap.NumMips();
    mOrder = bmap.Order();
}

void RndCubeTex::CubeTexProperties::Save(BinStream &bs) {
    bs << mBpp;
    bs << mHeight;
    bs << mWidth;
    bs << mNumMips;
    bs << mOrder;
}

void RndCubeTex::CubeTexProperties::Load(BinStream &bs) {
    bs >> mBpp >> mHeight >> mWidth >> mNumMips >> mOrder;
}

#pragma endregion
#pragma region RndCubeTex

RndCubeTex::RndCubeTex() {}

BEGIN_HANDLERS(RndCubeTex)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndCubeTex)
    SYNC_PROP_SET(
        right,
        FileRelativePath(FilePath::Root().c_str(), mFile[kCubeFaceRight].c_str()),
        SetBitmap(kCubeFaceRight, _val.Str(), true)
    )
    SYNC_PROP_SET(
        left,
        FileRelativePath(FilePath::Root().c_str(), mFile[kCubeFaceLeft].c_str()),
        SetBitmap(kCubeFaceLeft, _val.Str(), true)
    )
    SYNC_PROP_SET(
        top,
        FileRelativePath(FilePath::Root().c_str(), mFile[kCubeFaceTop].c_str()),
        SetBitmap(kCubeFaceTop, _val.Str(), true)
    )
    SYNC_PROP_SET(
        bottom,
        FileRelativePath(FilePath::Root().c_str(), mFile[kCubeFaceBottom].c_str()),
        SetBitmap(kCubeFaceBottom, _val.Str(), true)
    )
    SYNC_PROP_SET(
        front,
        FileRelativePath(FilePath::Root().c_str(), mFile[kCubeFaceFront].c_str()),
        SetBitmap(kCubeFaceFront, _val.Str(), true)
    )
    SYNC_PROP_SET(
        back,
        FileRelativePath(FilePath::Root().c_str(), mFile[kCubeFaceBack].c_str()),
        SetBitmap(kCubeFaceBack, _val.Str(), true)
    )
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndCubeTex)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    mBaseProps.Save(bs);
    for (int i = 0; i < kNumCubeFaces; i++) {
        mFaceProps[i].Save(bs);
    }
    for (int i = 0; i < kNumCubeFaces; i++) {
        bs << mFile[i];
    }
    if (bs.Cached()) {
        for (int i = 0; i < kNumCubeFaces; i++) {
            mBitmap[i].Save(bs);
        }
    }
END_SAVES

BEGIN_COPYS(RndCubeTex)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(RndCubeTex)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mBaseProps)
        for (int i = 0; i < kNumCubeFaces; i++) {
            COPY_MEMBER(mFaceProps[i])
            COPY_MEMBER(mFile[i])
            mBitmap[i].Create(
                c->mBitmap[i], c->mBitmap[i].Bpp(), c->mBitmap[i].Order(), 0
            );
        }
    END_COPYING_MEMBERS
    Update();
END_COPYS

void RndCubeTex::Load(BinStream &bs) {
    PreLoad(bs);
    PostLoad(bs);
}

INIT_REVS(2, 0)

void RndCubeTex::PreLoad(BinStream &bs) {
    LOAD_REVS(bs);
    ASSERT_REVS(2, 0);
    LOAD_SUPERCLASS(Hmx::Object)
    if (d.rev > 1) {
        if (bs.Cached()) {
            mBaseProps.Load(bs);
            for (int i = 0; i < kNumCubeFaces; i++) {
                mFaceProps[i].Load(bs);
            }
        } else {
            CubeTexProperties ctprops;
            ctprops.Load(bs);
            for (int i = 0; i < kNumCubeFaces; i++) {
                ctprops.Load(bs);
            }
        }
    } else {
        int i, j;
        bs >> i;
        bs >> j;
        mBaseProps.mWidth = i;
        mBaseProps.mHeight = i;
        mBaseProps.mBpp = j;
    }
    for (int i = 0; i < kNumCubeFaces; i++) {
        bs >> mFile[i];
        if (!bs.Cached()) {
            TheLoadMgr.AddLoader(mFile[i], kLoadFront);
        }
    }
    PUSH_REVS(d);
}

void RndCubeTex::PostLoad(BinStream &bs) {
    POP_REVS(bs);
    if (d.rev < 2) {
        bool b = false;
        d >> b;
    }
    for (int i = 0; i < kNumCubeFaces; i++) {
        if (bs.Cached()) {
            mBitmap[i].Load(bs);
        } else if (!mFile[i].empty()) {
            SetBitmap((CubeFace)i, mFile[i], false);
        }
        if (d.rev < 2) {
            mBaseProps = mFaceProps[i];
        }
    }
    Update();
}

bool RndCubeTex::LoadBitmap(const FilePath &fp, RndBitmap &bmap) {
    const char *cc;
    bool ret = true;
    FileLoader *fLoader = dynamic_cast<FileLoader *>(TheLoadMgr.ForceGetLoader(fp));
    if (fLoader) {
        cc = fLoader->GetBuffer(nullptr);
    } else
        cc = nullptr;

    delete fLoader;
    if (!cc) {
        bmap.Reset();
        ret = false;
    } else {
        bmap.Create((void *)cc);
        int width = bmap.Width();
        int height = bmap.Height();
        if (width != height) {
            MILO_NOTIFY(
                "%s: CubeTex bitmap (%s) is not square.", PathName(this), fp.c_str()
            );
            bmap.Reset();
            ret = false;
        }
    }
    return ret;
}

bool RndCubeTex::ValidateBitmapProperties(std::vector<CubeFace> &invalidFaces) {
    if (mBaseProps.mWidth == 0 || mBaseProps.mHeight == 0)
        return false;
    else {
        const char *sizeStr = RndTex::CheckSize(
            mBaseProps.mWidth,
            mBaseProps.mHeight,
            mBaseProps.mBpp,
            mBaseProps.mNumMips,
            RndTex::kRegular,
            false
        );
        if (sizeStr) {
            MILO_NOTIFY(sizeStr, Name());
            return false;
        } else {
            for (int i = 1; i < kNumCubeFaces; i++) {
                CubeTexProperties &propsToAnalyze = mFaceProps[i];
                if (mBaseProps.mWidth != propsToAnalyze.mWidth
                    || mBaseProps.mHeight != propsToAnalyze.mHeight
                    || mBaseProps.mBpp != propsToAnalyze.mBpp
                    || mBaseProps.mNumMips != propsToAnalyze.mNumMips
                    || mBaseProps.mOrder != propsToAnalyze.mOrder) {
                    invalidFaces.push_back((CubeFace)i);
                    return false;
                }
            }
            return true;
        }
    }
}

void RndCubeTex::Update() {
    std::vector<CubeFace> vec;
    if (!ValidateBitmapProperties(vec)) {
        Reset();
        return;
    } else {
        mBaseProps = mFaceProps[0];
        Sync();
        return;
    }
}

void RndCubeTex::SetBitmap(CubeFace face, const FilePath &fp, bool update) {
    if (LoadBitmap(fp, mBitmap[face])) {
        mFile[face] = fp;
        mFaceProps[face].Set(mBitmap[face]);
        mBaseProps = mFaceProps[face];
    }
    if (update)
        Update();
}

void RndCubeTex::UpdateFace(CubeFace face) {
    mFaceProps[face].Set(mBitmap[face]);
    mBaseProps = mFaceProps[face];
    Update();
}

#pragma endregion
