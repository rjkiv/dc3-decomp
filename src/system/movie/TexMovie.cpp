#include "movie/TexMovie.h"
#include "macros.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "os/Debug.h"
#include "os/File.h"
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"
#include "rndobj/Rnd.h"
#include "rndobj/Tex.h"
#include "rndobj/Utl.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include <cstddef>

TexMovie::TexMovie()
    : mTex(this), mLoop(true), unk5d(0), mLocalized(0), mPaused(0), mMovie() {}

TexMovie::~TexMovie() { mMovie.End(); }

bool TexMovie::Replace(ObjRef *from, Hmx::Object *to) {
    if (&mTex == from) {
        mMovie.End();
        mTex.SetObj(to);
        return true;
    } else {
        return Hmx::Object::Replace(from, to);
    }
}

BEGIN_HANDLERS(TexMovie)
    HANDLE(get_render_textures, OnGetRenderTextures)
    HANDLE(play_movie, OnPlayMovie)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(TexMovie)
    SYNC_PROP_MODIFY(output_texture, mTex, DoBeginMovieFromFile(nullptr, kLoadFront))
    SYNC_PROP_SET(
        bink_movie_file,
        FileRelativePath(FilePath::Root().c_str(), mBinkMovieFile.c_str()),
        SetFile(_val.Str())
    )
    SYNC_PROP(loop, mLoop)
    SYNC_PROP(is_localized, mLocalized)
    SYNC_PROP_SET(is_empty, mBinkMovieFile.empty(), )
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
    SYNC_SUPERCLASS(RndPollable)
END_PROPSYNCS

BEGIN_SAVES(TexMovie)
    SAVE_REVS(8, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    SAVE_SUPERCLASS(RndPollable)
    bs << mTex << mLoop << mBinkMovieFile << mLocalized;
    mMovie.Save(&bs);
END_SAVES

BEGIN_COPYS(TexMovie)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    COPY_SUPERCLASS(RndPollable)
    CREATE_COPY(TexMovie)
    BEGIN_COPYING_MEMBERS
        mTex = c->mTex.Ptr();
        COPY_MEMBER(mLoop)
        COPY_MEMBER(mBinkMovieFile)
        COPY_MEMBER(mLocalized)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(8, 0)

BEGIN_LOADS(TexMovie)
    LOAD_REVS(bs)
    ASSERT_REVS(8, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndDrawable)
    LOAD_SUPERCLASS(RndPollable)
    d.stream >> mTex >> mLoop;
    if (d.rev < 4) {
        bool b;
        d >> b;
    }
    d >> mBinkMovieFile;
    if (d.rev > 5) {
        d >> mLocalized;
    }
    if (d.rev == 7) {
        bool b;
        d >> b;
    }
    static Message msg("change_file");
    DataNode handled = HandleType(msg);
    if (handled.Type() == kDataString) {
        mBinkMovieFile.SetRoot(handled.Str());
    }
    if (d.rev > 1 && d.rev < 3) {
        bool b;
        d >> b;
    }
    FilePathTracker tracker(".");
    DoBeginMovieFromFile(d.rev > 4 ? &d.stream : nullptr, kLoadFront);
END_LOADS

void TexMovie::DrawPreClear() {
    if (mShowing)
        DrawToTexture();
}

void TexMovie::UpdatePreClearState() {
    if (unk5d) {
        TheRnd.PreClearDrawAddOrRemove(this, true, TheRnd.GetUnk1b4());
    }
}

void TexMovie::Poll() {
    if (!mPaused) {
        if (mShowing) {
            mMovie.SetPaused(false);
            if (mTex && !mMovie.Poll()) {
                mMovie.End();
            }
        } else {
            mMovie.SetPaused(true);
        }
    }
}

void TexMovie::Enter() {
    unk5d = true;
    RndPollable::Enter();
    if (HasTex()) {
        mTex->MakeDrawTarget();
        Hmx::Rect r(0, 0, 1, 1);
        Hmx::Color c(0, 0, 0, 1);
        TheRnd.DrawRectScreen(r, c, nullptr, nullptr, nullptr);
        mTex->FinishDrawTarget();
        TheRnd.MakeDrawTarget();
    }
    mMovie.CheckOpen(false);
    UpdatePreClearState();
}

void TexMovie::Exit() {
    unk5d = false;
    RndPollable::Exit();
}

void TexMovie::SetPaused(bool paused) {
    mPaused = paused;
    if (paused) {
        if (mMovie.IsOpen()) {
            mMovie.SetPaused(true);
        }
    } else {
        if (mMovie.IsOpen()) {
            mMovie.SetPaused(false);
        }
    }
}

void TexMovie::Reset() {
    mPaused = false;
    mMovie.End();
}

bool TexMovie::IsEmpty() const { return mBinkMovieFile.empty(); }

void TexMovie::DrawToTexture() {
    if (HasTex()) {
        mTex->MakeDrawTarget();
        mMovie.Draw();
        mTex->FinishDrawTarget();
        TheRnd.MakeDrawTarget();
    }
}

void TexMovie::SetFile(FilePath const &fp) {
    mMovie.End();
    mBinkMovieFile = fp;
    DoBeginMovieFromFile(0, kLoadBack);
}

void TexMovie::DoBeginMovieFromFile(BinStream *stream, LoaderPos lp) {
    mMovie.End();
    if (!mBinkMovieFile.empty() && mTex) {
        MILO_ASSERT(mTex->IsRenderTarget(), 0x83);
        int i = 1;
        if (mLocalized) {
            i = mMovie.LocalizationTrack();
        }
        mMovie.SetWidthHeight(mTex->Width(), mTex->Height());
        mMovie.BeginFromFile(
            FileRelativePath(FileRoot(), mBinkMovieFile.c_str()),
            0,
            0,
            mLoop,
            true,
            false,
            i,
            stream,
            lp
        );
    }
}

DataNode TexMovie::OnPlayMovie(DataArray *d) {
    if (d->Int(2) != 0) {
        bool b = mMovie.IsLoading(); // ok hmx
        if (!b && !mMovie.IsOpen())
            DoBeginMovieFromFile(0, kLoadFront);
    } else {
        mMovie.End();
    }
    return DataNode();
}

DataNode TexMovie::OnGetRenderTextures(DataArray *d) { return GetRenderTextures(Dir()); }
