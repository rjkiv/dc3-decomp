#include "movie/TexMovie.h"
#include "macros.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "os/Debug.h"
#include "os/File.h"
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"
#include "rndobj/Rnd.h"
#include "rndobj/Utl.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include <cstddef>

TexMovie::TexMovie()
    : mTex(this), unk5c(1), unk5d(0), unk5e(0), unk5f(0), sRoot(), mMovie() {}

TexMovie::~TexMovie() { mMovie.End(); }

BEGIN_COPYS(TexMovie)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    COPY_SUPERCLASS(RndPollable)
    CREATE_COPY(TexMovie)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mTex)
        COPY_MEMBER(sRoot)
        COPY_MEMBER(unk5e)
    END_COPYING_MEMBERS
END_COPYS

bool TexMovie::Replace(ObjRef *a, Hmx::Object *b) {
    bool check;
    if (&mTex == a) {
        mMovie.End();
        mTex.SetObj(b);
        check = true;
    } else {
        check = Hmx::Object::Replace(a, b);
    }
    return check;
}

BEGIN_PROPSYNCS(TexMovie)
    SYNC_PROP_MODIFY(output_texture, mTex, DoBeginMovieFromFile(nullptr, kLoadBack))
    SYNC_PROP_SET(bink_movie_file, sRoot, SetFile(sRoot.c_str()))
    SYNC_PROP(loop, unk5e)
    SYNC_PROP(is_localized, unk5d)
    SYNC_PROP(is_empty, sRoot)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
    SYNC_SUPERCLASS(RndPollable)
END_PROPSYNCS

BEGIN_SAVES(TexMovie)
    SAVE_REVS(8, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    SAVE_SUPERCLASS(RndPollable)
    bs << mTex << unk5c << sRoot << unk5e;
    mMovie.Save(&bs);
END_SAVES

BEGIN_LOADS(TexMovie)
    LOAD_REVS(bs)
    ASSERT_REVS(5, 1)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndDrawable)
    LOAD_SUPERCLASS(RndPollable)
    if (gAltRev < 4)
        bs >> unk5e;
    bs >> mTex >> unk5c;

END_LOADS

void TexMovie::DrawPreClear() {
    if (mShowing)
        DrawToTexture();
}

void TexMovie::UpdatePreClearState() {
    if (!unk5d)
        return;
    TheRnd.PreClearDrawAddOrRemove(this, true, TheRnd.GetUnk1b4());
}

void TexMovie::Poll() {
    if (unk5f == false) {
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
    bool b = (mTex && mTex->Width() && mTex->Height());
    if (b) {
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

void TexMovie::SetPaused(bool b) {
    unk5f = b;
    if (b) {
        if (!mMovie.IsOpen())
            return;
        mMovie.SetPaused(true);
    } else {
        if (!mMovie.IsOpen())
            return;
        mMovie.SetPaused(false);
    }
}

void TexMovie::Reset() {
    unk5f = false;
    mMovie.End();
}

bool TexMovie::IsEmpty() const { return sRoot.empty(); }

void TexMovie::DrawToTexture() {
    bool b = (mTex != nullptr && mTex->Width() && mTex->Height());

    if (b) {
        mTex->MakeDrawTarget();
        mMovie.Draw();
        mTex->FinishDrawTarget();
        TheRnd.MakeDrawTarget();
    }
}

void TexMovie::SetFile(FilePath const &fp) {
    mMovie.End();
    sRoot = fp;
    DoBeginMovieFromFile(0, kLoadBack);
}

void TexMovie::DoBeginMovieFromFile(BinStream *stream, LoaderPos lp) {
    mMovie.End();
    if (!sRoot.empty() && mTex) {
        MILO_ASSERT(mTex->IsRenderTarget(), 0x83);
        int i = 1;
        if (unk5e) {
            i = mMovie.LocalizationTrack();
        }
        mMovie.SetWidthHeight(mTex->Width(), mTex->Height());
        mMovie.BeginFromFile(
            FileRelativePath(FileRoot(), sRoot.c_str()),
            0.0f,
            0,
            true,
            unk5c,
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

BEGIN_HANDLERS(TexMovie)
    HANDLE(get_render_textures, OnGetRenderTextures)
    HANDLE(play_movie, OnPlayMovie)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
