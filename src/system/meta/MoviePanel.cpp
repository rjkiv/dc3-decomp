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

bool MoviePanel::sUseSubtitles;

MoviePanel::MoviePanel()
    : mMovies(), mRecent(), unk58(), unk60(0), unk64(0), unk68(0), unk6c(1), unk70(0),
      unk74(0), unk78(0), unk7c(0.0f), unk80(0) {}

bool MoviePanel::IsLoaded() const {
    if (!unk58.Ready()) {
        return false;
    }
    if (unk60 && !unk60->IsLoaded()) {
        return false;
    }
    return UIPanel::IsLoaded();
}

void MoviePanel::Enter() { UIPanel::Enter(); }

void MoviePanel::Exit() {
    UIPanel::Exit();
    if (unk38 == false) {
        unk58.End();
    }
    unk80 = false;
}

void MoviePanel::Draw() {
    if (GetState() != kUnloaded && unk34 == GetFinalDrawPass())
        unk58.Draw();
    UIPanel::Draw();
}

void MoviePanel::FinishLoad() {
    UIPanel::FinishLoad();
    if (unk60) {
        unk64 = unk60->Data();
        unk64->AddRef();
        RELEASE(unk60);
        if (mDir)
            unk70 = mDir->Find<UILabel>("subtitles.lbl", false);
    } else {
        unk64 = 0;
    }
    if (mDir) {
        unk74 = mDir->Find<RndAnimatable>("fade_pausehint.anim", true);
    }
}

void MoviePanel::Unload() {
    UIPanel::Unload();
    if (unk38) {
        unk58.End();
    }
    if (unk64) {
        unk64->Release();
        unk64 = nullptr;
    }
}

void MoviePanel::SetTypeDef(DataArray *d) {
    UIPanel::SetTypeDef(d);
    d->FindData("preload", unk38, true);
    d->FindData("audio", unk39, true);
    d->FindData("loop", unk3a, true);
}

void MoviePanel::Poll() {
    UIPanel::Poll();
    if (GetState() == kUnloaded)
        return;
    if (!unk58.Poll() && !TheUI->InTransition()) {
        if (unk64 && unk70) {
        }
    }
}

void MoviePanel::Load() {
    UIPanel::Load();
    mMovies.clear();

    DataArray *config = SystemConfig("videos", Property("videos", true)->Str());

    DataArray *files = config->FindArray("files");
    for (int i = 1; i < files->Size(); i++) {
        mMovies.push_back(files->Str(i));
    }

    unk3b = false;
    config->FindData("fill_width", unk3b, false);

    bool localize = false;
    config->FindData("localize", localize, false);
    if (localize) {
        unk3c = false;
    } else {
        unk3c = Movie::LocalizationTrack();
    }

    if (sUseSubtitles && mMovies.size() == 1) {
        char pathBuffer[256];
        // sprintf(pathBuffer, "ui/subtitles/eng/%s_keep.dta", FileGetBase(mMovies[0]));
        const char *subtitlesPath;
        if (FileIsLocal(pathBuffer) && UsingCD()) {
            subtitlesPath = MakeString(
                "%s/gen/%s.dtb", FileGetPath(pathBuffer), FileGetBase(pathBuffer)
            );
        } else {
            subtitlesPath = pathBuffer;
        }

        if (FileExists(subtitlesPath, 0, nullptr)) {
            // unk60 = new DataLoader(FilePath(subtitlesPath), kLoadFront, true);
        }
    }

    ChooseMovie();
}

void MoviePanel::SetPaused(bool b) { unk58.SetPaused(b); }

void MoviePanel::HideHint() {
    unk78 = false;
    unk74->Animate(
        unk74->GetFrame(),
        unk74->StartFrame(),
        unk74->Units(),
        0,
        0.0f,
        0,
        kEaseLinear,
        0.0f,
        0
    );
}

void MoviePanel::ShowHint() {
    if (unk74) {
        unk78 = true;
        unk7c = TheTaskMgr.UISeconds();
        unk74->Animate(
            unk74->GetFrame(),
            unk74->EndFrame(),
            unk74->Units(),
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
    unk6c = true;
    unk68 = 0;
    unk58.BeginFromFile(
        MakeString("videos/%s", mCurrentMovie),
        0.0f,
        unk39 == 0,
        unk3a,
        unk38,
        unk3b,
        unk3c,
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
    unk80 = b;
    if (unk80)
        HideHint();
}

BEGIN_PROPSYNCS(MoviePanel)
    SYNC_PROP(show_menu, unk80)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_HANDLERS(MoviePanel)
    HANDLE_ACTION(set_paused, SetPaused(_msg->Int(2)))
    HANDLE_ACTION(set_menu_shown, ShowMenu(_msg->Int(2)))
    HANDLE_EXPR(is_menu_shown, unk80)
    HANDLE_ACTION(show_hint, ShowHint())
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS
