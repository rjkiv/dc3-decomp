#include "rndobj/CubeTex.h"
#include "CubeTex.h"
#include "os/File.h"
#include "rndobj/Tex.h"

#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"

void RndCubeTex::CubeTexProperties::Set(const RndBitmap &bmap) {
    mBpp = bmap.Bpp();
    mHeight = bmap.Height();
    mWidth = bmap.Width();
    mNumMips = bmap.NumMips();
    mOrder = bmap.Order();
}

void RndCubeTex::Load(BinStream &bs) {
    PreLoad(bs);
    PostLoad(bs);
}

void RndCubeTex::CubeTexProperties::Load(BinStream &bs) {
    bs >> mBpp >> mHeight >> mWidth >> mNumMips >> mOrder;
}

void RndCubeTex::CubeTexProperties::Save(BinStream &bs) {
    bs << mBpp;
    bs << mHeight;
    bs << mWidth;
    bs << mNumMips;
    bs << mOrder;
}

RndCubeTex::RndCubeTex() {}

bool RndCubeTex::LoadBitmap(const FilePath &fp, RndBitmap &bmap) {
    const char *cc;
    bool ret = true;
    FileLoader *fLoader = dynamic_cast<FileLoader *>(TheLoadMgr.ForceGetLoader(fp));
    if (fLoader) {
        cc = fLoader->GetBuffer(0);
    } else
        cc = 0;

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

void RndCubeTex::Save(BinStream &bs) {
    bs << 2;
    SAVE_SUPERCLASS(Hmx::Object)
    props.Save(bs);
    for (int i = 0; i < kNumCubeFaces; i++) {
        moreprops[i].Save(bs);
    }
    for (int i = 0; i < kNumCubeFaces; i++) {
        bs << mFile[i];
    }
    if (bs.Cached()) {
        for (int i = 0; i < kNumCubeFaces; i++) {
            mBitmap[i].Save(bs);
        }
    }
}

void RndCubeTex::PreLoad(BinStream &bs) {
    LOAD_REVS(bs);
    ASSERT_REVS(2, 0);
    LOAD_SUPERCLASS(Hmx::Object)
    if (gRev > 1) {
        if (bs.Cached()) {
            props.Load(bs);
            for (int i = 0; i < kNumCubeFaces; i++) {
                moreprops[i].Load(bs);
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
        props.mWidth = i;
        props.mHeight = i;
        props.mBpp = j;
    }
    for (int i = 0; i < kNumCubeFaces; i++) {
        bs >> mFile[i];
        if (!bs.Cached())
            TheLoadMgr.AddLoader(mFile[i], kLoadFront);
    }
    bsrev.PushRev(this);
}

bool RndCubeTex::ValidateBitmapProperties(std::vector<CubeFace> &faces) {
    if (props.mWidth == 0 || props.mHeight == 0)
        return false;
    else {
        const char *sizeStr = RndTex::CheckSize(
            props.mWidth,
            props.mHeight,
            props.mBpp,
            props.mNumMips,
            RndTex::kTexRegular,
            false
        );
        if (sizeStr) {
            MILO_NOTIFY(sizeStr, Name());
            return false;
        } else {
            for (int i = 1; i < kNumCubeFaces; i++) {
                CubeTexProperties &propsToAnalyze = moreprops[i];
                if (props.mWidth != propsToAnalyze.mWidth
                    || props.mHeight != propsToAnalyze.mHeight
                    || props.mBpp != propsToAnalyze.mBpp
                    || props.mNumMips != propsToAnalyze.mNumMips
                    || props.mOrder != propsToAnalyze.mOrder) {
                    faces.push_back((CubeFace)i);
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
        props = moreprops[0];
        Sync();
        return;
    }
}

BEGIN_COPYS(RndCubeTex)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(RndCubeTex)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(props)
        for (int i = 0; i < kNumCubeFaces; i++) {
            COPY_MEMBER(moreprops[i])
            COPY_MEMBER(mFile[i])
            mBitmap[i].Create(
                c->mBitmap[i], c->mBitmap[i].Bpp(), c->mBitmap[i].Order(), 0
            );
        }
    END_COPYING_MEMBERS
    Update();
END_COPYS

void RndCubeTex::SetBitmap(CubeFace face, const FilePath &fp, bool b) {
    if (LoadBitmap(fp, mBitmap[face])) {
        mFile[face] = fp;
        moreprops[face].Set(mBitmap[face]);
        props = moreprops[face];
    }
    if (b)
        Update();
}

void RndCubeTex::UpdateFace(CubeFace face) {
    moreprops[face].Set(mBitmap[face]);
    props = moreprops[face];
    Update();
}

void RndCubeTex::PostLoad(BinStream &bs) {
    BinStreamRev bsrev(bs, bs.PopRev(this));
    if (bsrev.mRev < 2) {
        bool b;
        bsrev >> b;
    }
    for (int i = 0; i < kNumCubeFaces; i++) {
        if (bs.Cached()) {
            mBitmap[i].Load(bs);
        } else if (!mFile[i].empty()) {
            SetBitmap((CubeFace)i, mFile[i], false);
        }
        if (bsrev.mRev < 2) {
            props = moreprops[i];
        }
    }
    Update();
}

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
