#include "rndobj/Movie.h"
#include "obj/Object.h"
#include "os/File.h"
#include "rndobj/Anim.h"
#include "rndobj/Tex.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"

RndMovie::RndMovie() : mStream(false), mLoop(true), mTex(this) {}

bool RndMovie::Replace(ObjRef *from, Hmx::Object *to) {
    if (&mTex == from) {
        SetTex(dynamic_cast<RndTex *>(to));
        return true;
    } else {
        return Hmx::Object::Replace(from, to);
    }
}

BEGIN_HANDLERS(RndMovie)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndMovie)
    SYNC_PROP_SET(
        movie_file,
        FileRelativePath(FilePath::Root().c_str(), mFile.c_str()),
        SetFile(_val.Str(), mStream)
    )
    SYNC_PROP_SET(stream, mStream, SetFile(mFile, _val.Int()))
    SYNC_PROP(loop, mLoop)
    SYNC_PROP_SET(tex, mTex.Ptr(), SetTex(_val.Obj<RndTex>()))
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndMovie)
    SAVE_REVS(8, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndAnimatable)
    bs << mFile << mTex << mStream << mLoop;
END_SAVES

BEGIN_COPYS(RndMovie)
    CREATE_COPY_AS(RndMovie, t);
    MILO_ASSERT(t, 0x55);
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    COPY_MEMBER_FROM(t, mLoop)
    mTex = t->mTex.Ptr();
    SetFile(t->mFile, t->mStream);
END_COPYS

BEGIN_LOADS(RndMovie)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

INIT_REVS(8, 0)

void RndMovie::PreLoad(BinStream &bs) {
    LOAD_REVS(bs);
    ASSERT_REVS(8, 0);
    if (d.rev > 6)
        LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndAnimatable)
    d >> mFile;
    if (d.rev > 3)
        d >> mTex;
    if (d.rev > 4)
        d >> mStream;
    if (d.rev > 7 && !mStream) {
        TheLoadMgr.AddLoader(mFile, kLoadFront);
    }
    PUSH_REVS(d);
}

void RndMovie::PostLoad(BinStream &bs) {
    POP_REVS(bs);
    if (d.rev > 5) {
        d >> mLoop;
    } else if (d.rev == 5) {
        mLoop = !mStream;
    }
    SetFile(mFile, mStream);
}

void RndMovie::SetTex(RndTex *tex) { mTex = tex; }
