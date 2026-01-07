#include "meta/MoviePanel.h"
#include "macros.h"
#include "math/Easing.h"
#include "math/Rand.h"
#include "net/json-c/printbuf.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "rndobj/Rnd.h"
#include "ui/PanelDir.h"
#include "ui/UI.h"
#include "ui/UILabel.h"
#include "ui/UIPanel.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/MakeString.h"
#include "utl/Symbol.h"
#include <cstdio>

bool MoviePanel::sUseSubtitles;

#pragma region Hmx::Object

MoviePanel::MoviePanel()
    : mMovies(), mRecent(), mMovie(), mSubtitlesLoader(0), mSubtitles(0),
      mCurrentSubtitleIndex(0), mSubtitleCleared(1), mSubtitleLabel(0), mPauseHintAnim(0),
      mShowHint(0), mTimeShowHintStarted(0.0f), mShowMenu(0) {}

BEGIN_HANDLERS(MoviePanel)
    HANDLE_ACTION(set_paused, SetPaused(_msg->Int(2)))
    HANDLE_ACTION(set_menu_shown, ShowMenu(_msg->Int(2)))
    HANDLE_EXPR(is_menu_shown, mShowMenu)
    HANDLE_ACTION(show_hint, ShowHint())
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS

BEGIN_PROPSYNCS(MoviePanel)
    SYNC_PROP(show_menu, mShowMenu)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void MoviePanel::SetTypeDef(DataArray *d) {
    UIPanel::SetTypeDef(d);
    d->FindData("preload", mPreload, true);
    d->FindData("audio", mAudio, true);
    d->FindData("loop", mLoop, true);
}

#pragma endregion
#pragma region UIPanel

void MoviePanel::Load() {
    UIPanel::Load();
    mMovies.clear();
    DataArray *config = SystemConfig("videos", Property("videos", true)->Str());
    DataArray *files = config->FindArray("files");
    for (int i = 1; i < files->Size(); i++) {
        mMovies.push_back(files->Str(i));
    }
    mFillWidth = false;
    config->FindData("fill_width", mFillWidth, false);
    bool localize = false;
    config->FindData("localize", localize, false);
    if (localize) {
        mLanguage = Movie::LocalizationTrack();
    } else {
        mLanguage = 0;
    }
    if (sUseSubtitles && mMovies.size() == 1) {
        char pathBuffer[256];
        sprintf(pathBuffer, "ui/subtitles/eng/%s_keep.dta", FileGetBase(mMovies[0]));
        const char *subtitlesPath;
        bool local = FileIsLocal(pathBuffer);
        bool cd = UsingCD();
        if (!cd || local) {
            subtitlesPath = pathBuffer;
        } else {
            subtitlesPath = MakeString(
                "%s/gen/%s.dtb", FileGetPath(pathBuffer), FileGetBase(pathBuffer)
            );
        }

        if (FileExists(subtitlesPath, 0, nullptr)) {
            // bug? pathBuffer should probably be subtitlesPath
            mSubtitlesLoader = new DataLoader(pathBuffer, kLoadFront, true);
        }
    }
    ChooseMovie();
}

void MoviePanel::Draw() {
    if (GetState() != kUnloaded && unk34 == GetFinalDrawPass())
        mMovie.Draw();
    UIPanel::Draw();
}

void MoviePanel::Enter() { UIPanel::Enter(); }

void MoviePanel::Exit() {
    UIPanel::Exit();
    if (mPreload == false) {
        mMovie.End();
    }
    mShowMenu = false;
}

void MoviePanel::Poll() {
    UIPanel::Poll();
    if (GetState() == kUnloaded)
        return;
    if (!mMovie.Poll() && !TheUI->InTransition()) {
        static Message movie_done("movie_done");
        DataNode handled = HandleType(movie_done);
        if (handled.Equal(DATA_UNHANDLED, nullptr, true)) {
            mMovie.End();
            PlayMovie();
        }
    } else {
        if (mSubtitles && mSubtitleLabel) {
            int frame = mMovie.GetFrame();
            DataArray *arr = mSubtitles->Array(mCurrentSubtitleIndex);
            if (mSubtitleCleared) {
                if (arr->Int(0) <= frame) {
                    mSubtitleLabel->SetSubtitle(arr);
                    mSubtitleCleared = false;
                }
            }
            if (arr->Int(1) < frame) {
                if (mSubtitles->Size() > mCurrentSubtitleIndex + 1) {
                    DataArray *a2 = arr->Array(mCurrentSubtitleIndex + 1);
                    if (a2) {
                        if (a2->Int(0) <= frame) {
                            mSubtitleLabel->SetSubtitle(a2);
                            mSubtitleCleared = false;
                            mCurrentSubtitleIndex++;
                            goto lol;
                        }
                    }
                }
                if (!mSubtitleCleared) {
                    mSubtitleLabel->SetTextToken(gNullStr);
                    mSubtitleCleared = true;
                }
            }
        }
    }
lol:
    if (mShowHint) {
        float secs = TheTaskMgr.UISeconds();
        if (secs < mTimeShowHintStarted) {
            mTimeShowHintStarted = secs;
        }
        if (secs - mTimeShowHintStarted >= 3.0f) {
            HideHint();
        }
    }
}

bool MoviePanel::IsLoaded() const {
    if (!mMovie.Ready()) {
        return false;
    }
    if (mSubtitlesLoader && !mSubtitlesLoader->IsLoaded()) {
        return false;
    }
    return UIPanel::IsLoaded();
}

void MoviePanel::Unload() {
    UIPanel::Unload();
    if (mPreload) {
        mMovie.End();
    }
    if (mSubtitles) {
        mSubtitles->Release();
        mSubtitles = nullptr;
    }
}

void MoviePanel::FinishLoad() {
    UIPanel::FinishLoad();
    if (mSubtitlesLoader) {
        mSubtitles = mSubtitlesLoader->Data();
        mSubtitles->AddRef();
        RELEASE(mSubtitlesLoader);
        if (mDir)
            mSubtitleLabel = mDir->Find<UILabel>("subtitles.lbl", false);
    } else {
        mSubtitles = 0;
    }
    if (mDir) {
        mPauseHintAnim = mDir->Find<RndAnimatable>("fade_pausehint.anim", true);
    }
}

#pragma endregion
#pragma region MoviePanel

void MoviePanel::SetPaused(bool b) { mMovie.SetPaused(b); }

void MoviePanel::HideHint() {
    mShowHint = false;
    mPauseHintAnim->Animate(
        mPauseHintAnim->GetFrame(),
        mPauseHintAnim->StartFrame(),
        mPauseHintAnim->Units(),
        0,
        0.0f,
        0,
        kEaseLinear,
        0.0f,
        0
    );
}

void MoviePanel::ShowHint() {
    if (mPauseHintAnim) {
        mShowHint = true;
        mTimeShowHintStarted = TheTaskMgr.UISeconds();
        mPauseHintAnim->Animate(
            mPauseHintAnim->GetFrame(),
            mPauseHintAnim->EndFrame(),
            mPauseHintAnim->Units(),
            0.0f,
            0.0f,
            0,
            kEaseLinear,
            0.0f,
            false
        );
    }
}

void MoviePanel::PlayMovie() {
    mSubtitleCleared = true;
    mCurrentSubtitleIndex = 0;
    mMovie.BeginFromFile(
        MakeString("videos/%s", mCurrentMovie),
        0.0f,
        mAudio == 0,
        mLoop,
        mPreload,
        mFillWidth,
        mLanguage,
        0,
        kLoadFront
    );
}

void MoviePanel::ChooseMovie() {
    MILO_ASSERT(!mMovies.empty(), 0xf2);
    do {
        int random = RandomInt(0, mMovies.size());
        mCurrentMovie = mMovies[random];
    } while (std::find(mRecent.begin(), mRecent.end(), mCurrentMovie) != mRecent.end());

    mRecent.push_back(mCurrentMovie);

    if (mRecent.size() == mMovies.size()) {
        while (mRecent.size() > (mMovies.size() / 2)) {
            mRecent.pop_front();
        }
    }
    PlayMovie();
}

void MoviePanel::ShowMenu(bool b) {
    mShowMenu = b;
    if (mShowMenu)
        HideHint();
}
